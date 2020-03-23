// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ZIRCON_SYSTEM_DEV_AUDIO_VIM3_TDM_OUTPUT_AUDIO_STREAM_OUT_H_
#define ZIRCON_SYSTEM_DEV_AUDIO_VIM3_TDM_OUTPUT_AUDIO_STREAM_OUT_H_

#include <threads.h>

#include <lib/device-protocol/pdev.h>
#include <lib/fzl/pinned-vmo.h>
#include <lib/simple-audio-stream/simple-audio-stream.h>
#include <lib/zx/bti.h>
#include <lib/zx/vmo.h>

#include <optional>

#include <memory>

#include <audio-proto/audio-proto.h>
#include <ddk/io-buffer.h>
#include <ddk/protocol/platform/device.h>
#include <ddktl/device-internal.h>
#include <ddktl/device.h>
#include <fbl/mutex.h>
#include <soc/aml-common/aml-tdm-audio.h>

namespace audio {
namespace vim3 {

class Vim3AudioStreamOut;
using Vim3AudioStreamType = ddk::Device<Vim3AudioStreamOut, ddk::Initializable>;

class Vim3AudioStreamOut : public Vim3AudioStreamType {
 public:

  Vim3AudioStreamOut(zx_device_t* parent) : Vim3AudioStreamType(parent) {}

  static zx_status_t Create(void* ctx, zx_device_t* parent);
  void DdkInit(ddk::InitTxn txn);
  void DdkRelease() {}

 protected:

  zx_status_t Start(uint64_t* out_start_time);

 private:
  friend class fbl::RefPtr<Vim3AudioStreamOut>;

  static constexpr uint8_t kFifoDepth = 0x20;

  int Thread();


  zx_status_t InitBuffer(size_t size);
  zx_status_t InitPDev();

  std::optional<ddk::InitTxn> init_txn_;

  uint32_t us_per_notification_ = 0;

  ddk::PDev pdev_;

  zx::vmo ring_buffer_vmo_;
  fzl::PinnedVmo pinned_ring_buffer_;

  std::unique_ptr<AmlTdmDevice> aml_audio_;

  zx::bti bti_;
  thrd_t thread_;

};

}  // namespace vim3
}  // namespace audio

#endif  // ZIRCON_SYSTEM_DEV_AUDIO_VIM3_TDM_OUTPUT_AUDIO_STREAM_OUT_H_
