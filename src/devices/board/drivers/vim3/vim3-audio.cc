// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ddk/binding.h>
#include <ddk/debug.h>
#include <ddk/device.h>
#include <ddk/platform-defs.h>
#include <ddk/protocol/platform/bus.h>
#include <soc/aml-a311d/a311d-gpio.h>
#include <soc/aml-a311d/a311d-hw.h>
#include <soc/aml-meson/g12b-clk.h>

#include "vim3.h"

namespace vim3 {

static const pbus_mmio_t audio_mmios[] = {
    {.base = A311D_EE_AUDIO_BASE, .length = A311D_EE_AUDIO_LENGTH},
};

static const pbus_bti_t tdm_btis[] = {
    {
        .iommu_index = 0,
        .bti_id = BTI_AUDIO_OUT,
    },
};

static pbus_dev_t tdm_dev = []() {
  pbus_dev_t dev = {};
  dev.name = "vim3-tdm";
  dev.vid = PDEV_VID_KHADAS;
  dev.pid = PDEV_PID_VIM3;
  dev.did = PDEV_DID_AMLOGIC_TDM;
  dev.mmio_list = audio_mmios;
  dev.mmio_count = countof(audio_mmios);
  dev.bti_list = tdm_btis;
  dev.bti_count = countof(tdm_btis);
  return dev;
}();

static const zx_bind_inst_t root_match[] = {
    BI_MATCH(),
};

static const zx_bind_inst_t fault_gpio_match[] = {
    BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_GPIO),
    BI_MATCH_IF(EQ, BIND_GPIO_PIN, A311D_GPIOH(4)),  // Audio fault gpio
};

static const zx_bind_inst_t enable_gpio_match[] = {
    BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_GPIO),
    BI_MATCH_IF(EQ, BIND_GPIO_PIN, A311D_GPIOA(0)),  // Audio En gpio
};

static const device_fragment_part_t fault_gpio_fragment[] = {
    {countof(root_match), root_match},
    {countof(fault_gpio_match), fault_gpio_match},
};

static const device_fragment_part_t enable_gpio_fragment[] = {
    {countof(root_match), root_match},
    {countof(enable_gpio_match), enable_gpio_match},
};

static constexpr zx_bind_inst_t audio_clock_match[] = {
    BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_CLOCK),
    BI_MATCH_IF(EQ, BIND_CLOCK_ID, g12b_clk::CLK_MP0_DDS.clock_id),
};
static const device_fragment_part_t audio_clock_fragment[] = {
    {std::size(root_match), root_match},
    {std::size(audio_clock_match), audio_clock_match},
};

static const device_fragment_t fragments[] = {
    {countof(fault_gpio_fragment), fault_gpio_fragment},
    {countof(enable_gpio_fragment), enable_gpio_fragment},
    {countof(audio_clock_fragment), audio_clock_fragment},
};

zx_status_t Vim3::AudioInit() {
  zx_status_t status;

  // TDM pin assignments
  gpio_impl_.SetAltFunction(A311D_GPIOA(1), A311D_GPIOA_1_TDMB_SCLK_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOA(2), A311D_GPIOA_2_TDMB_FS_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOA(3), A311D_GPIOA_3_TDMB_D0_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOA(4), A311D_GPIOA_4_TDMB_DIN1_FN);
  constexpr uint8_t medium_strength = 2;  // Not mA, unitless values from AMLogic.
  gpio_impl_.SetDriveStrength(A311D_GPIOA(1), medium_strength);
  gpio_impl_.SetDriveStrength(A311D_GPIOA(2), medium_strength);
  gpio_impl_.SetDriveStrength(A311D_GPIOA(3), medium_strength);

  gpio_impl_.ConfigOut(A311D_GPIOA(0), 0);
  gpio_impl_.ConfigIn(A311D_GPIOH(4), GPIO_PULL_UP);

  status = pbus_.CompositeDeviceAdd(&tdm_dev, fragments, countof(fragments), UINT32_MAX);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s: CompositeDeviceAdd failed: %d\n", __func__, status);
    return status;
  }
  return ZX_OK;
}

}  // namespace vim3
