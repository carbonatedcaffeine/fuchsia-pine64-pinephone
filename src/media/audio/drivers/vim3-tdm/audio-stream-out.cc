// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "audio-stream-out.h"

#include <lib/zx/clock.h>
#include <math.h>

#include <optional>
#include <utility>

#include <ddk/binding.h>
#include <ddk/debug.h>
#include <ddk/metadata.h>
#include <ddk/platform-defs.h>
#include <ddk/protocol/composite.h>
#include <fbl/auto_call.h>

namespace audio {
namespace vim3 {

enum {
  FRAGMENT_PDEV,
  FRAGMENT_FAULT_GPIO,
  FRAGMENT_ENABLE_GPIO,
  FRAGMENT_AUDIO_CLOCK,
  FRAGMENT_COUNT,
};

constexpr size_t kNumberOfChannels = 1;
constexpr size_t kMinSampleRate = 48000;
constexpr size_t kMaxSampleRate = 96000;
constexpr size_t kBytesPerSample = 2;
// Calculate ring buffer size for 1 second of 16-bit, max rate.
constexpr size_t kRingBufferSize =
    fbl::round_up<size_t, size_t>(kMaxSampleRate * kBytesPerSample * kNumberOfChannels, PAGE_SIZE);

Vim3AudioStream::Vim3AudioStream(zx_device_t* parent) : SimpleAudioStream(parent, false) {
  dai_format_.number_of_channels = kNumberOfChannels;
  for (size_t i = 0; i < kNumberOfChannels; ++i) {
    dai_format_.channels_to_use.push_back(1 << i);  // Use all channels.
  }
  dai_format_.sample_format = SAMPLE_FORMAT_PCM_SIGNED;
  dai_format_.justify_format = JUSTIFY_FORMAT_JUSTIFY_I2S;
  dai_format_.frame_rate = kMinSampleRate;
  dai_format_.bits_per_sample = 16;
  dai_format_.bits_per_channel = 32;
}

zx_status_t Vim3AudioStream::InitHW() {
  zx_status_t status;

  // Shut down the SoC audio peripherals (tdm/dma)
  aml_audio_->Shutdown();

  auto on_error = fbl::MakeAutoCall([this]() { aml_audio_->Shutdown(); });

  aml_audio_->Initialize();
  // Setup TDM.

  // bitoffset = 3, 1 slot, 16 bits/slot, 16 bits/sample.
  // bitoffest = 3 places msb of sample one sclk period after fsync to provide PCM framing.
  aml_audio_->ConfigTdmOutSlot(3, 0, 15, 15, 0);

  // Lane 0, unmask first slot1 (0x00000001),
  status = aml_audio_->ConfigTdmOutLane(0, 0x00000001, 0);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not configure TDM out lane %d", __FILE__, status);
    return status;
  }

  // PLL sourcing audio clock tree should be running at 768MHz
  // Note: Audio clock tree input should always be < 1GHz
  // mclk rate for 96kHz = 768MHz/5 = 153.6MHz
  // mclk rate for 48kHz = 768MHz/10 = 76.8MHz
  // Note: absmax mclk frequency is 500MHz per AmLogic
  uint32_t mdiv = 1;                          //(dai_format_.frame_rate == 96000) ? 5 : 10;
  status = aml_audio_->SetMclkDiv(mdiv - 1);  // register val is div - 1;
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not configure MCLK %d", __FILE__, status);
    return status;
  }

  // No need to set mclk pad via SetMClkPad (TAS2770 features "MCLK Free Operation").

  // 48kHz: sclk=76.8MHz/25 = 3.072MHz, 3.072MHz/64=48kkHz
  // 96kHz: sclk=153.6MHz/25 = 6.144MHz, 6.144MHz/64=96kHz
  // lrduty = 1 sclk cycles (write 0) for PCM
  // TODO(andresoportus): For now we set lrduty to 2 sclk cycles (write 1), 1 does not work.
  // invert sclk = false = sclk is falling edge in middle of bit for PCM
  status = aml_audio_->SetSclkDiv(3, 1, 15, false);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not configure SCLK %d", __FILE__, status);
    return status;
  }

  // Allow clock divider changes to stabilize
  zx_nanosleep(zx_deadline_after(ZX_MSEC(1)));

  aml_audio_->Sync();

  on_error.cancel();
  // At this point the SoC audio peripherals are ready to start, but no
  //  clocks are active.
  return ZX_OK;
}

zx_status_t Vim3AudioStream::InitPDev() {
  composite_protocol_t composite;

  auto status = device_get_protocol(parent(), ZX_PROTOCOL_COMPOSITE, &composite);
  if (status != ZX_OK) {
    zxlogf(ERROR, "Could not get composite protocol");
    return status;
  }

  size_t actual = 0;

  zx_device_t* fragments[FRAGMENT_COUNT] = {};
  composite_get_fragments(&composite, fragments, countof(fragments), &actual);
  // Either we have all fragments (for I2S) or we have only one fragment (for PCM).

  pdev_ = fragments[FRAGMENT_PDEV];
  if (!pdev_.is_valid()) {
    return ZX_ERR_NO_RESOURCES;
  }

  status = pdev_.GetBti(0, &bti_);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not obtain bti - %d", __func__, status);
    return status;
  }

  std::optional<ddk::MmioBuffer> mmio;
  status = pdev_.MapMmio(0, &mmio);
  if (status != ZX_OK) {
    return status;
  }

  audio_clock_ = ddk::ClockProtocolClient(fragments[FRAGMENT_AUDIO_CLOCK]);
  if (!audio_clock_.is_valid()) {
    zxlogf(ERROR, "%s: Failed to get clock protocol", __func__);
    return ZX_ERR_NO_RESOURCES;
  }

  aml_audio_ = AmlTdmDevice::Create(*std::move(mmio), MP0_PLL, TDM_OUT_A, FRDDR_A, MCLK_A);
  if (aml_audio_ == nullptr) {
    zxlogf(ERROR, "%s failed to create TDM device", __func__);
    return ZX_ERR_NO_MEMORY;
  }
  status = audio_clock_.SetRate(6144000);
  if (status != ZX_OK) {
    zxlogf(ERROR, "Setrate failes %d", status);
  }

  // Initialize the ring buffer
  status = InitBuffer(kRingBufferSize);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to init buffer %d", __FILE__, status);
    return status;
  }

  status = aml_audio_->SetBuffer(pinned_ring_buffer_.region(0).phys_addr,
                                 pinned_ring_buffer_.region(0).size);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to set buffer %d", __FILE__, status);
    return status;
  }

  status = InitHW();
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to init tdm hardware %d\n", __FILE__, status);
    return status;
  }

  zxlogf(INFO, "audio: vim3 audio output initialized");
  return ZX_OK;
}

zx_status_t Vim3AudioStream::Init() {
  zx_status_t status;

  status = InitPDev();
  if (status != ZX_OK) {
    return status;
  }

  status = AddFormats();
  if (status != ZX_OK) {
    return status;
  }

  // Set our gain capabilities.

  {
    cur_gain_state_.cur_gain = 1.f;
    cur_gain_state_.cur_mute = false;
    cur_gain_state_.cur_agc = false;

    cur_gain_state_.min_gain = 1.f;
    cur_gain_state_.max_gain = 1.f;
    cur_gain_state_.gain_step = .0f;
    cur_gain_state_.can_mute = false;
    cur_gain_state_.can_agc = false;
  }

  snprintf(device_name_, sizeof(device_name_), "vim3-audio-out");
  unique_id_ = AUDIO_STREAM_UNIQUE_ID_BUILTIN_SPEAKERS;
  snprintf(mfr_name_, sizeof(mfr_name_), "Spacely Sprockets");
  snprintf(prod_name_, sizeof(prod_name_), "vim3");

  // TODO(mpuryear): change this to the domain of the clock received from the board driver
  uint32_t domain = 0;
  status = audio_clock_.GetDomain(&domain);
  if (status != ZX_OK) {
    zxlogf(ERROR, "GetDomain failed: %d", status);
    return status;
  }
  clock_domain_ = domain;
  zxlogf(INFO, "*************************************Clock domain = %u", domain);
  return ZX_OK;
}

// Timer handler for sending out position notifications
void Vim3AudioStream::ProcessRingNotification() {
  ScopedToken t(domain_token());
  if (us_per_notification_) {
    notify_timer_.PostDelayed(dispatcher(), zx::usec(us_per_notification_));
  } else {
    notify_timer_.Cancel();
    return;
  }

  audio_proto::RingBufPositionNotify resp = {};
  resp.hdr.cmd = AUDIO_RB_POSITION_NOTIFY;

  resp.monotonic_time = zx::clock::get_monotonic().get();
  resp.ring_buffer_pos = aml_audio_->GetRingPosition();
  NotifyPosition(resp);
}

zx_status_t Vim3AudioStream::ChangeFormat(const audio_proto::StreamSetFmtReq& req) {
  fifo_depth_ = aml_audio_->fifo_depth();
  external_delay_nsec_ = 0;  // Unknown.

  if (req.frames_per_second != dai_format_.frame_rate) {
    uint32_t last_rate = dai_format_.frame_rate;
    dai_format_.frame_rate = req.frames_per_second;
    auto status = InitHW();
    if (status != ZX_OK) {
      dai_format_.frame_rate = last_rate;
      return status;
    }
  }
  return ZX_OK;
}

void Vim3AudioStream::ShutdownHook() {
  aml_audio_->Shutdown();
  pinned_ring_buffer_.Unpin();
}

zx_status_t Vim3AudioStream::SetGain(const audio_proto::SetGainReq& req) { return ZX_OK; }

zx_status_t Vim3AudioStream::GetBuffer(const audio_proto::RingBufGetBufferReq& req,
                                       uint32_t* out_num_rb_frames, zx::vmo* out_buffer) {
  uint32_t rb_frames = static_cast<uint32_t>(pinned_ring_buffer_.region(0).size) / frame_size_;

  if (req.min_ring_buffer_frames > rb_frames) {
    return ZX_ERR_OUT_OF_RANGE;
  }
  zx_status_t status;
  constexpr uint32_t rights = ZX_RIGHT_READ | ZX_RIGHT_WRITE | ZX_RIGHT_MAP | ZX_RIGHT_TRANSFER;
  status = ring_buffer_vmo_.duplicate(rights, out_buffer);
  if (status != ZX_OK) {
    return status;
  }

  *out_num_rb_frames = rb_frames;

  aml_audio_->SetBuffer(pinned_ring_buffer_.region(0).phys_addr, rb_frames * frame_size_);

  return ZX_OK;
}

zx_status_t Vim3AudioStream::Start(uint64_t* out_start_time) {
  *out_start_time = aml_audio_->Start();

  uint32_t notifs = LoadNotificationsPerRing();
  if (notifs) {
    us_per_notification_ =
        static_cast<uint32_t>(1000 * pinned_ring_buffer_.region(0).size /
                              (frame_size_ * dai_format_.frame_rate / 1000 * notifs));
    notify_timer_.PostDelayed(dispatcher(), zx::usec(us_per_notification_));
  } else {
    us_per_notification_ = 0;
  }
  return ZX_OK;
}

zx_status_t Vim3AudioStream::Stop() {
  notify_timer_.Cancel();
  us_per_notification_ = 0;
  aml_audio_->Stop();
  return ZX_OK;
}

zx_status_t Vim3AudioStream::AddFormats() {
  fbl::AllocChecker ac;
  supported_formats_.reserve(1, &ac);
  if (!ac.check()) {
    zxlogf(ERROR, "Out of memory, can not create supported formats list");
    return ZX_ERR_NO_MEMORY;
  }

  // Add the range for basic audio support.
  audio_stream_format_range_t range;

  range.min_channels = kNumberOfChannels;
  range.max_channels = kNumberOfChannels;
  range.sample_formats = AUDIO_SAMPLE_FORMAT_16BIT;
  range.min_frames_per_second = kMinSampleRate;
  range.max_frames_per_second = kMaxSampleRate;
  range.flags = ASF_RANGE_FLAG_FPS_48000_FAMILY;

  supported_formats_.push_back(range);

  return ZX_OK;
}

zx_status_t Vim3AudioStream::InitBuffer(size_t size) {
  // Make sure the DMA is stopped before releasing quarantine.
  aml_audio_->Stop();
  // Make sure that all reads/writes have gone through.
#if defined(__aarch64__)
  asm __volatile__("dsb sy");
#endif
  auto status = bti_.release_quarantine();
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not release quarantine bti - %d", __func__, status);
    return status;
  }
  status = zx_vmo_create_contiguous(bti_.get(), size, 0, ring_buffer_vmo_.reset_and_get_address());
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to allocate ring buffer vmo - %d", __func__, status);
    return status;
  }

  status = pinned_ring_buffer_.Pin(ring_buffer_vmo_, bti_, ZX_VM_PERM_READ | ZX_VM_PERM_WRITE);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to pin ring buffer vmo - %d", __func__, status);
    return status;
  }
  if (pinned_ring_buffer_.region_count() != 1) {
    zxlogf(ERROR, "%s buffer is not contiguous", __func__);
    return ZX_ERR_NO_MEMORY;
  }

  return ZX_OK;
}

static zx_status_t audio_bind(void* ctx, zx_device_t* device) {
  auto stream = audio::SimpleAudioStream::Create<audio::vim3::Vim3AudioStream>(device);
  if (stream == nullptr) {
    return ZX_ERR_NO_MEMORY;
  }

  __UNUSED auto dummy = fbl::ExportToRawPtr(&stream);

  return ZX_OK;
}

static constexpr zx_driver_ops_t driver_ops = []() {
  zx_driver_ops_t ops = {};
  ops.version = DRIVER_OPS_VERSION;
  ops.bind = audio_bind;
  return ops;
}();

}  // namespace vim3
}  // namespace audio

// clang-format off
ZIRCON_DRIVER_BEGIN(aml_tdm, audio::vim3::driver_ops, "aml-tdm-out", "0.1", 4)
    BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_COMPOSITE),
    BI_ABORT_IF(NE, BIND_PLATFORM_DEV_VID, PDEV_VID_KHADAS),
    BI_ABORT_IF(NE, BIND_PLATFORM_DEV_PID, PDEV_PID_VIM3),
    BI_MATCH_IF(EQ, BIND_PLATFORM_DEV_DID, PDEV_DID_AMLOGIC_TDM),
ZIRCON_DRIVER_END(aml_tdm)
    // clang-format on
