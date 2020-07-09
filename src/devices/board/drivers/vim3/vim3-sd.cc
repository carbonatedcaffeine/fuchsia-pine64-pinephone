// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ddk/binding.h>
#include <ddk/debug.h>
#include <ddk/metadata.h>
#include <ddk/platform-defs.h>
#include <ddk/protocol/sdmmc.h>
#include <soc/aml-a311d/a311d-gpio.h>
#include <soc/aml-a311d/a311d-hw.h>
#include <soc/aml-common/aml-sdmmc.h>

#include "vim3.h"

namespace vim3 {

static const pbus_mmio_t sd_mmios[] = {
    {
        .base = A311D_EMMC_B_BASE,
        .length = A311D_EMMC_B_LENGTH,
    },
};

static const pbus_irq_t sd_irqs[] = {
    {
        .irq = A311D_SD_EMMC_B_IRQ,
        .mode = ZX_INTERRUPT_MODE_EDGE_HIGH,
    },
};

static const pbus_bti_t sd_btis[] = {
    {
        .iommu_index = 0,
        .bti_id = BTI_SD,
    },
};

static aml_sdmmc_config_t config = {
    .supports_dma = true,
    .min_freq = 400'000,
    .max_freq = 50'000'000,
    .version_3 = true,
    .prefs = 0x1000'0000,  // Magic number to detect the SD slot.
};

static const pbus_metadata_t sd_metadata[] = {
    {
        .type = DEVICE_METADATA_PRIVATE,
        .data_buffer = &config,
        .data_size = sizeof(config),
    },
};

static const zx_bind_inst_t root_match[] = {
    BI_MATCH(),
};
static const zx_bind_inst_t i2c_match[] = {
      BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_I2C),
      BI_ABORT_IF(NE, BIND_I2C_BUS_ID, 0),
      BI_MATCH_IF(EQ, BIND_I2C_ADDRESS, 0x20),
};
static const device_fragment_part_t i2c_fragment[] = {
    {countof(root_match), root_match},
    {countof(i2c_match), i2c_match},
};
static const device_fragment_t fragments[] = {
    {countof(i2c_fragment), i2c_fragment},
};

zx_status_t Vim3::SdInit() {
  zx_status_t status;

  pbus_dev_t sd_dev = {};
  sd_dev.name = "aml_sd";
  sd_dev.vid = PDEV_VID_AMLOGIC;
  sd_dev.pid = PDEV_PID_GENERIC;
  sd_dev.did = PDEV_DID_AMLOGIC_SDMMC_B;
  sd_dev.mmio_list = sd_mmios;
  sd_dev.mmio_count = countof(sd_mmios);
  sd_dev.irq_list = sd_irqs;
  sd_dev.irq_count = countof(sd_irqs);
  sd_dev.bti_list = sd_btis;
  sd_dev.bti_count = countof(sd_btis);
  sd_dev.metadata_list = sd_metadata;
  sd_dev.metadata_count = countof(sd_metadata);

  gpio_impl_.SetAltFunction(A311D_GPIOC(0), A311D_GPIOC_0_SDCARD_D0_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOC(1), A311D_GPIOC_1_SDCARD_D1_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOC(2), A311D_GPIOC_2_SDCARD_D2_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOC(3), A311D_GPIOC_3_SDCARD_D3_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOC(4), A311D_GPIOC_4_SDCARD_CLK_FN);
  gpio_impl_.SetAltFunction(A311D_GPIOC(5), A311D_GPIOC_5_SDCARD_CMD_FN);

  if ((status = pbus_.CompositeDeviceAdd(&sd_dev, fragments, countof(fragments), UINT32_MAX)) !=
      ZX_OK) {
    zxlogf(ERROR, "SdInit could not add sd_dev: %d", status);
    return status;
  }

  return ZX_OK;
}

}  // namespace vim3
