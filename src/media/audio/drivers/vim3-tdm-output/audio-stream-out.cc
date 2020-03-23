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
#include <ddk/driver.h>
#include <ddk/platform-defs.h>

namespace audio {
namespace vim3 {

enum {
  COMPONENT_PDEV,
  COMPONENT_COUNT,
};

// Calculate ring buffer size for 1 second of 16-bit, 48kHz, stereo.
constexpr size_t RB_SIZE = fbl::round_up<size_t, size_t>(48000 * 2 * 2u, PAGE_SIZE);

zx_status_t Vim3AudioStreamOut::Create(void* ctx, zx_device_t* parent) {
#if 0
  pdev_protocol_t pdev;

  auto status = device_get_protocol(parent(), ZX_PROTOCOL_PDEV, &pdev);
  if (status) {
    return status;
  }

  status = pdev_.GetBti(0, &bti_);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not obtain bti - %d\n", __func__, status);
    return status;
  }

  std::optional<ddk::MmioBuffer> mmio;
  status = pdev_.MapMmio(0, &mmio);
  if (status != ZX_OK) {
    return status;
  }
#endif
  fbl::AllocChecker ac;
  auto tdm_out = fbl::make_unique_checked<Vim3AudioStreamOut>(&ac, parent);
  if (!ac.check()) {
    return ZX_ERR_NO_MEMORY;
  }

  zx_status_t status = tdm_out->DdkAdd("vim3-tdm-output", DEVICE_ADD_INVISIBLE);
  if (status != ZX_OK) {
    return status;
  }

  if (status == ZX_OK) {
    // devmgr is now in charge of the device.
    __UNUSED auto* dummy = tdm_out.release();
  }

  return status;
}


int Vim3AudioStreamOut::Thread() {

  auto status = device_get_protocol(parent(), ZX_PROTOCOL_PDEV, &pdev_);
  if (status) {
    return status;
  }

  status = pdev_.GetBti(0, &bti_);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s could not obtain bti - %d\n", __func__, status);
    return status;
  }

  std::optional<ddk::MmioBuffer> mmio;
  status = pdev_.MapMmio(0, &mmio);
  if (status != ZX_OK) {
    return status;
  }
  if (!pdev_.is_valid()) {
    return ZX_ERR_NO_RESOURCES;
  }


  aml_audio_ = AmlTdmDevice::Create(*std::move(mmio), HIFI_PLL, TDM_OUT_B, FRDDR_B, MCLK_B);
  if (aml_audio_ == nullptr) {
    zxlogf(ERROR, "%s failed to create tdm device\n", __func__);
    return ZX_ERR_NO_MEMORY;
  }

  // Initialize the ring buffer
  InitBuffer(RB_SIZE);

  aml_audio_->SetBuffer(pinned_ring_buffer_.region(0).phys_addr,
                        pinned_ring_buffer_.region(0).size);

  // Setup TDM.

  // 3 bitoffset, 4 slots, 32 bits/slot, 16 bits/sample, no mixing.
  aml_audio_->ConfigTdmOutSlot(3, 1, 31, 15, 0);

  // Lane0 right channel.
  aml_audio_->ConfigTdmOutSwaps(0x00000010);

  // Lane 0, unmask first 2 slots (0x00000003),
  aml_audio_->ConfigTdmOutLane(0, 0x00000003);
//TODO - these are initial values for first stage of fpga testing.  First cut of
//       verilog required a minimum sclk of 10MHz, so clock rates here deviate from
//       other audio driver implementations in zircon.
  // Setup appropriate tdm clock signals. mclk = 1536MHz/62 = 24.774MHz.
  aml_audio_->SetMclkDiv(124);

  // No need to set mclk pad via SetMClkPad (TAS2770 features "MCLK Free Operation").

  // sclk = 24.774MHz/2 = 12.387MHz, 1 every 128 sclks is frame sync.
  aml_audio_->SetSclkDiv(1, 0, 63, false);

  aml_audio_->Sync();
  aml_audio_->Start();

  init_txn_->Reply(ZX_OK);
  return ZX_OK;
}



zx_status_t Vim3AudioStreamOut::Start(uint64_t* out_start_time) {
  *out_start_time = aml_audio_->Start();
  return ZX_OK;
}


zx_status_t Vim3AudioStreamOut::InitBuffer(size_t size) {
  zx_status_t status;
  status = zx_vmo_create_contiguous(bti_.get(), size, 0, ring_buffer_vmo_.reset_and_get_address());
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to allocate ring buffer vmo - %d\n", __func__, status);
    return status;
  }

  status = pinned_ring_buffer_.Pin(ring_buffer_vmo_, bti_, ZX_VM_PERM_READ | ZX_VM_PERM_WRITE);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s failed to pin ring buffer vmo - %d\n", __func__, status);
    return status;
  }
  if (pinned_ring_buffer_.region_count() != 1) {
    zxlogf(ERROR, "%s buffer is not contiguous", __func__);
    return ZX_ERR_NO_MEMORY;
  }
  uint16_t l=0;
  for (size_t i=0; i < RB_SIZE; i=i+4) {
    ring_buffer_vmo_.write(&l, i, 2);
    ring_buffer_vmo_.write(&l, i + 2, 2);
    l++;
  }

  return ZX_OK;
}


void Vim3AudioStreamOut::DdkInit(ddk::InitTxn txn) {
  init_txn_ = std::move(txn);
  int rc = thrd_create_with_name(
      &thread_, [](void* arg) -> int { return reinterpret_cast<Vim3AudioStreamOut*>(arg)->Thread(); }, this,
      "vim3-start-thread");
  if (rc != thrd_success) {
    init_txn_->Reply(ZX_ERR_INTERNAL);
  }
}

static constexpr zx_driver_ops_t driver_ops = []() {
  zx_driver_ops_t ops = {};
  ops.version = DRIVER_OPS_VERSION;
  ops.bind = Vim3AudioStreamOut::Create;
  return ops;
}();

}  // namespace vim3
}  // namespace audio

// clang-format off
ZIRCON_DRIVER_BEGIN(aml_tdm, audio::vim3::driver_ops, "aml-tdm-out", "0.1", 3)
    BI_ABORT_IF(NE, BIND_PLATFORM_DEV_VID, PDEV_VID_KHADAS),
    BI_ABORT_IF(NE, BIND_PLATFORM_DEV_PID, PDEV_PID_VIM3),
    BI_MATCH_IF(EQ, BIND_PLATFORM_DEV_DID, PDEV_DID_TEST_TDM),
ZIRCON_DRIVER_END(aml_tdm)
    // clang-format on
