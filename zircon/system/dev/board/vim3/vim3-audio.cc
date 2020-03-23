// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ddk/binding.h>
#include <ddk/debug.h>
#include <ddk/device.h>
#include <ddk/platform-defs.h>
#include <ddk/protocol/platform/bus.h>
#include <soc/aml-meson/g12b-clk.h>
#include <soc/aml-a311d/a311d-gpio.h>
#include <soc/aml-a311d/a311d-hw.h>

#include "vim3.h"
#include "vim3-gpios.h"

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
  dev.name = "Vim3TdmOut";
  dev.vid = PDEV_VID_KHADAS;
  dev.pid = PDEV_PID_VIM3;
  dev.did = PDEV_DID_TEST_TDM;
  dev.mmio_list = audio_mmios;
  dev.mmio_count = countof(audio_mmios);
  dev.bti_list = tdm_btis;
  dev.bti_count = countof(tdm_btis);
  return dev;
}();


zx_status_t Vim3::AudioInit() {
  zx_status_t status;
#if 1
  status = clk_impl_.Disable(g12b_clk::CLK_HIFI_PLL);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s: Disable(CLK_HIFI_PLL) failed, st = %d\n",
           __func__, status);
    return status;
  }

  status = clk_impl_.SetRate(g12b_clk::CLK_HIFI_PLL, 1536000000);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s: SetRate(CLK_HIFI_PLL) failed, st = %d\n",
           __func__, status);
    return status;
  }

  status = clk_impl_.Enable(g12b_clk::CLK_HIFI_PLL);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s: Enable(CLK_HIFI_PLL) failed, st = %d\n",
           __func__, status);
    return status;
  }
#endif
  // TDM pin assignments
  gpio_impl_.SetAltFunction(A311D_GPIOA(1), A311D_GPIOA_1_TDMB_SCLK_FN);
  gpio_impl_.SetDriveStrength(A311D_GPIOA(1), 3);

  gpio_impl_.SetAltFunction(A311D_GPIOA(2), A311D_GPIOA_2_TDMB_FS_FN);
  gpio_impl_.SetDriveStrength(A311D_GPIOA(2), 3);

  gpio_impl_.SetAltFunction(A311D_GPIOA(3), A311D_GPIOA_3_TDMB_D0_FN);
  gpio_impl_.SetDriveStrength(A311D_GPIOA(3), 3);

  gpio_impl_.SetAltFunction(A311D_GPIOA(6), A311D_GPIOA_6_TDMB_DIN3_FN);

  status = pbus_.DeviceAdd(&tdm_dev);
  if (status != ZX_OK) {
    zxlogf(ERROR, "%s: DeviceAdd failed: %d\n", __func__, status);
    return status;
  }

  return ZX_OK;
}

}  // namespace vim3
