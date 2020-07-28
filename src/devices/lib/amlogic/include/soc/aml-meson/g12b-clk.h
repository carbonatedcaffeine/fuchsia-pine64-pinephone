// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef SRC_DEVICES_LIB_AMLOGIC_INCLUDE_SOC_AML_MESON_G12B_CLK_H_
#define SRC_DEVICES_LIB_AMLOGIC_INCLUDE_SOC_AML_MESON_G12B_CLK_H_

#include <ddk/metadata/clock.h>
#include <soc/aml-meson/aml-clk-common.h>
#include <soc/aml-s905d2/s905d2-hiu.h>

namespace g12b_clk {

using clk_type = ::aml_clk_common::aml_clk_type;
using clk_domain = ::aml_clk_common::aml_clk_domain;

// SYS CPU CLK
constexpr uint32_t G12B_CLK_SYS_PLL_DIV16 = AmlClkId(0, clk_type::kMesonGate);
constexpr uint32_t G12B_CLK_SYS_CPU_CLK_DIV16 = AmlClkId(1, clk_type::kMesonGate);

// GPIO 24MHz
constexpr uint32_t G12B_CLK_CAM_INCK_24M = AmlClkId(2, clk_type::kMesonGate);

// SYS CPUB CLK
constexpr uint32_t G12B_CLK_SYS_PLLB_DIV16 = AmlClkId(3, clk_type::kMesonGate);
constexpr uint32_t G12B_CLK_SYS_CPUB_CLK_DIV16 = AmlClkId(4, clk_type::kMesonGate);

constexpr uint32_t G12B_CLK_DOS_GCLK_VDEC = AmlClkId(5, clk_type::kMesonGate);

// NB: This must be the last entry
constexpr uint32_t CLK_G12B_COUNT = 6;

// kMesonPllClocks
constexpr uint32_t CLK_GP0_PLL = AmlClkId(GP0_PLL, clk_type::kMesonPll);
constexpr uint32_t CLK_PCIE_PLL = AmlClkId(PCIE_PLL, clk_type::kMesonPll);
constexpr uint32_t CLK_HIFI_PLL = AmlClkId(HIFI_PLL, clk_type::kMesonPll);
constexpr uint32_t CLK_SYS_PLL = AmlClkId(SYS_PLL, clk_type::kMesonPll);
constexpr uint32_t CLK_SYS1_PLL = AmlClkId(SYS1_PLL, clk_type::kMesonPll);

// kMesonDdsClocks
constexpr clock_id_t CLK_MP0_DDS =
    AmlClkId(MP0_DDS, clk_type::kMesonDds, clk_domain::kMpll0ClockDomain);
constexpr clock_id_t CLK_MP1_DDS =
    AmlClkId(MP1_DDS, clk_type::kMesonDds, clk_domain::kMpll1ClockDomain);
constexpr clock_id_t CLK_MP2_DDS =
    AmlClkId(MP2_DDS, clk_type::kMesonDds, clk_domain::kMpll2ClockDomain);
constexpr clock_id_t CLK_MP3_DDS =
    AmlClkId(MP3_DDS, clk_type::kMesonDds, clk_domain::kMpll3ClockDomain);

}  // namespace g12b_clk

#endif  // SRC_DEVICES_LIB_AMLOGIC_INCLUDE_SOC_AML_MESON_G12B_CLK_H_
