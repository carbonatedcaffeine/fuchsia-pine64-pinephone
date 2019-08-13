// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddk/mmio-buffer.h>
#include <ddk/protocol/platform/bus.h>
#include <hw/reg.h>

#define SDM_FRACTIONALITY ((uint32_t)16384)
#define A113_FIXED_PLL_RATE ((uint32_t)2000000000)
#define A113_CLOCKS_BASE_PHYS 0xff63c000

// Clock register offsets (all are 32-bit registers)
#define A113_HHI_MPLL_CNTL (0xa0)
#define A113_HHI_MPLL_CNTL8 (0xa8)
#define A113_HHI_PLL_TOP_MISC (0xba)

typedef struct {
  mmio_buffer_t mmio;
  uint32_t *regs_vaddr;
} a113_clk_dev_t;

static inline uint32_t a113_clk_get_reg(a113_clk_dev_t *dev, uint32_t offset) {
  return readl(dev->regs_vaddr + offset);
}

static inline uint32_t a113_clk_set_reg(a113_clk_dev_t *dev, uint32_t offset, uint32_t value) {
  writel(value, dev->regs_vaddr + offset);
  return a113_clk_get_reg(dev, offset);
}

zx_status_t a113_clk_init(a113_clk_dev_t **device);
zx_status_t a113_clk_set_mpll2(a113_clk_dev_t *device, uint64_t rate, uint64_t *actual);
