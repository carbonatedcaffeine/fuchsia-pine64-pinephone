// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tas27xx.h"

#include <lib/fake_ddk/fake_ddk.h>
#include <lib/mock-i2c/mock-i2c.h>
#include <lib/simple-codec/simple-codec-client.h>
#include <lib/simple-codec/simple-codec-helper.h>
#include <lib/sync/completion.h>

#include <mock/ddktl/protocol/gpio.h>
#include <zxtest/zxtest.h>

namespace audio {

audio::DaiFormat GetDefaultDaiFormat() {
  return {
      .number_of_channels = 2,
      .channels_to_use_bitmask = 1,  // Use one channel in this mono codec.
      .sample_format = SampleFormat::PCM_SIGNED,
      .frame_format = FrameFormat::I2S,
      .frame_rate = 24'000,
      .bits_per_slot = 32,
      .bits_per_sample = 16,
  };
}

struct Tas27xxCodec : public Tas27xx {
  explicit Tas27xxCodec(ddk::I2cChannel i2c, ddk::GpioProtocolClient fault)
      : Tas27xx(fake_ddk::kFakeParent, std::move(i2c), std::move(fault), true, true) {}
  codec_protocol_t GetProto() { return {&this->codec_protocol_ops_, this}; }
};

TEST(Tas27xxTest, CodecInitGood) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

TEST(Tas27xxTest, CodecInitBad) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  // Error when getting the interrupt.
  mock_fault.ExpectGetInterrupt(ZX_ERR_INTERNAL, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NULL(codec);
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
}

TEST(Tas27xxTest, CodecGetInfo) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);
  auto codec_proto = codec->GetProto();
  SimpleCodecClient client;
  client.SetProtocol(&codec_proto);
  auto info = client.GetInfo();
  ASSERT_EQ(info->unique_id.compare(""), 0);
  ASSERT_EQ(info->manufacturer.compare("Texas Instruments"), 0);
  ASSERT_EQ(info->product_name.compare("TAS2770"), 0);

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

TEST(Tas27xxTest, CodecReset) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  // Reset by the call to Reset.
  mock_i2c
      .ExpectWriteStop({0x01, 0x01})  // SW_RESET.
      .ExpectWriteStop({0x02, 0x0d})  // PRW_CTL stopped.
      .ExpectWriteStop({0x3c, 0x10})  // CLOCK_CFG.
      .ExpectWriteStop({0x0a, 0x07})  // SetRate.
      .ExpectWriteStop({0x0c, 0x22})  // TDM_CFG2.
      .ExpectWriteStop({0x0e, 0x02})  // TDM_CFG4.
      .ExpectWriteStop({0x0f, 0x44})  // TDM_CFG5.
      .ExpectWriteStop({0x10, 0x40})  // TDM_CFG6.
      .ExpectWrite({0x24})
      .ExpectReadStop({0x00})  // INT_LTCH0.
      .ExpectWrite({0x25})
      .ExpectReadStop({0x00})  // INT_LTCH1.
      .ExpectWrite({0x26})
      .ExpectReadStop({0x00})          // INT_LTCH2.
      .ExpectWriteStop({0x20, 0xf8})   // INT_MASK0.
      .ExpectWriteStop({0x21, 0xff})   // INT_MASK1.
      .ExpectWriteStop({0x30, 0x01})   // INT_CFG.
      .ExpectWriteStop({0x05, 0x3c})   // -30dB.
      .ExpectWriteStop({0x02, 0x0d});  // PWR_CTL stopped.

  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);
  auto codec_proto = codec->GetProto();
  SimpleCodecClient client;
  client.SetProtocol(&codec_proto);
  ASSERT_OK(client.Reset());

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

TEST(Tas27xxTest, CodecBridgedMode) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);
  auto codec_proto = codec->GetProto();
  SimpleCodecClient client;
  client.SetProtocol(&codec_proto);
  {
    auto bridgeable = client.IsBridgeable();
    ASSERT_FALSE(bridgeable.value());
  }
  client.SetBridgedMode(false);

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

TEST(Tas27xxTest, CodecDaiFormat) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);
  auto codec_proto = codec->GetProto();
  SimpleCodecClient client;
  client.SetProtocol(&codec_proto);

  // Check getting DAI formats.
  {
    auto formats = client.GetDaiFormats();
    ASSERT_EQ(formats->size(), 1);
    ASSERT_EQ(formats.value()[0].number_of_channels.size(), 1);
    ASSERT_EQ(formats.value()[0].number_of_channels[0], 2);
    ASSERT_EQ(formats.value()[0].sample_formats.size(), 1);
    ASSERT_EQ(formats.value()[0].sample_formats[0], SampleFormat::PCM_SIGNED);
    ASSERT_EQ(formats.value()[0].frame_formats.size(), 1);
    ASSERT_EQ(formats.value()[0].frame_formats[0], FrameFormat::I2S);
    ASSERT_EQ(formats.value()[0].frame_rates.size(), 2);
    ASSERT_EQ(formats.value()[0].frame_rates[0], 48000);
    ASSERT_EQ(formats.value()[0].frame_rates[1], 96000);
    ASSERT_EQ(formats.value()[0].bits_per_slot.size(), 1);
    ASSERT_EQ(formats.value()[0].bits_per_slot[0], 32);
    ASSERT_EQ(formats.value()[0].bits_per_sample.size(), 1);
    ASSERT_EQ(formats.value()[0].bits_per_sample[0], 16);
  }

  // Check setting DAI formats.
  {
    mock_i2c.ExpectWriteStop({0x0a, 0x07});
    DaiFormat format = GetDefaultDaiFormat();
    format.frame_rate = 48'000;
    auto formats = client.GetDaiFormats();
    ASSERT_TRUE(IsDaiFormatSupported(format, formats.value()));
    ASSERT_OK(client.SetDaiFormat(std::move(format)));
  }

  {
    mock_i2c.ExpectWriteStop({0x0a, 0x09});
    DaiFormat format = GetDefaultDaiFormat();
    format.frame_rate = 96'000;
    auto formats = client.GetDaiFormats();
    ASSERT_TRUE(IsDaiFormatSupported(format, formats.value()));
    ASSERT_OK(client.SetDaiFormat(std::move(format)));
  }

  {
    DaiFormat format = GetDefaultDaiFormat();
    format.frame_rate = 192'000;
    auto formats = client.GetDaiFormats();
    ASSERT_FALSE(IsDaiFormatSupported(format, formats.value()));
    ASSERT_NOT_OK(client.SetDaiFormat(std::move(format)));
  }

  // Make a 2-wal call to make sure the server (we know single threaded) completed previous calls.
  auto unused = client.GetInfo();
  static_cast<void>(unused);

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

TEST(Tas27xxTest, CodecGain) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);
  auto codec_proto = codec->GetProto();
  SimpleCodecClient client;
  client.SetProtocol(&codec_proto);

  mock_i2c
      .ExpectWriteStop({0x05, 0x40})   // -32dB.
      .ExpectWriteStop({0x02, 0x0d});  // PWR_CTL stopped.
  client.SetGainState({
      .gain = -32.f,
      .muted = false,
      .agc_enable = false,
  });

  // Lower than min gain.
  mock_i2c
      .ExpectWriteStop({0x05, 0xc8})   // -100dB.
      .ExpectWriteStop({0x02, 0x0d});  // PWR_CTL stopped.
  client.SetGainState({
      .gain = -999.f,
      .muted = false,
      .agc_enable = false,
  });

  // Higher than max gain.
  mock_i2c
      .ExpectWriteStop({0x05, 0x0})    // 0dB.
      .ExpectWriteStop({0x02, 0x0d});  // PWR_CTL stopped.
  client.SetGainState({
      .gain = 111.f,
      .muted = false,
      .agc_enable = false,
  });

  // Make a 2-wal call to make sure the server (we know single threaded) completed previous calls.
  auto unused = client.GetInfo();
  static_cast<void>(unused);

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

TEST(Tas27xxTest, CodecPlugState) {
  fake_ddk::Bind tester;
  zx::interrupt irq;
  ASSERT_OK(zx::interrupt::create(zx::resource(), 0, ZX_INTERRUPT_VIRTUAL, &irq));

  mock_i2c::MockI2c mock_i2c;
  ddk::MockGpio mock_fault;
  mock_fault.ExpectGetInterrupt(ZX_OK, ZX_INTERRUPT_MODE_EDGE_LOW, std::move(irq));

  auto codec = SimpleCodecServer::Create<Tas27xxCodec>(mock_i2c.GetProto(), mock_fault.GetProto());
  ASSERT_NOT_NULL(codec);
  auto codec_proto = codec->GetProto();
  SimpleCodecClient client;
  client.SetProtocol(&codec_proto);

  auto state = client.GetPlugState();
  ASSERT_TRUE(state->hardwired);
  ASSERT_TRUE(state->plugged);

  codec->DdkAsyncRemove();
  codec.release()->DdkRelease();  // codec release managed by the DDK
  mock_i2c.VerifyAndClear();
  mock_fault.VerifyAndClear();
  ASSERT_TRUE(tester.Ok());
}

}  // namespace audio
