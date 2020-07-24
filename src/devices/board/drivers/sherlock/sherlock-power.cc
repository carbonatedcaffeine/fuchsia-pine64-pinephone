// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ddk/binding.h>
#include <ddk/debug.h>
#include <ddk/device.h>
#include <ddk/metadata.h>
#include <ddk/metadata/power.h>
#include <ddk/platform-defs.h>
#include <soc/aml-common/aml-power.h>
#include <soc/aml-t931/t931-power.h>
#include <soc/aml-t931/t931-pwm.h>
#include <soc/aml-t931/t931-gpio.h>

#include "sherlock.h"

namespace sherlock {

namespace {

constexpr aml_voltage_table_t kT931VoltageTable[] = {
    {1'022'000, 0}, {1'011'000, 3}, {1'001'000, 6}, {991'000, 10}, {981'000, 13},
    {971'000, 16},  {961'000, 20},  {951'000, 23},  {941'000, 26}, {931'000, 30},
    {921'000, 33},  {911'000, 36},  {901'000, 40},  {891'000, 43}, {881'000, 46},
    {871'000, 50},  {861'000, 53},  {851'000, 56},  {841'000, 60}, {831'000, 63},
    {821'000, 67},  {811'000, 70},  {801'000, 73},  {791'000, 76}, {781'000, 80},
    {771'000, 83},  {761'000, 86},  {751'000, 90},  {741'000, 93}, {731'000, 96},
    {721'000, 100},
};

constexpr voltage_pwm_period_ns_t kT931PwmPeriodNs = 1250;

constexpr pbus_metadata_t power_impl_metadata[] = {
    {
        .type = DEVICE_METADATA_AML_VOLTAGE_TABLE,
        .data_buffer = &kT931VoltageTable,
        .data_size = sizeof(kT931VoltageTable),
    },
    {
        .type = DEVICE_METADATA_AML_PWM_PERIOD_NS,
        .data_buffer = &kT931PwmPeriodNs,
        .data_size = sizeof(kT931PwmPeriodNs),
    },
};

constexpr zx_bind_inst_t root_match[] = {
    BI_MATCH(),
};

constexpr zx_bind_inst_t power_impl_driver_match[] = {
    BI_MATCH_IF(EQ, BIND_PROTOCOL, ZX_PROTOCOL_POWER_IMPL),
};

constexpr device_fragment_part_t power_impl_fragment[] = {
    {countof(root_match), root_match},
    {countof(power_impl_driver_match), power_impl_driver_match},
};

zx_device_prop_t power_domain_arm_core_props[] = {
    {BIND_POWER_DOMAIN_COMPOSITE, 0, PDEV_DID_POWER_DOMAIN_COMPOSITE},
};

constexpr device_fragment_t power_domain_arm_core_fragments[] = {
    {countof(power_impl_fragment), power_impl_fragment},
};

constexpr power_domain_t big_domain[] = {
    {static_cast<uint32_t>(T931PowerDomains::kArmCoreBig)},
};

constexpr device_metadata_t power_domain_big_core[] = {
    {
        .type = DEVICE_METADATA_POWER_DOMAINS,
        .data = &big_domain,
        .length = sizeof(big_domain),
    },
};

constexpr composite_device_desc_t power_domain_big_core_desc = {
    .props = power_domain_arm_core_props,
    .props_count = countof(power_domain_arm_core_props),
    .fragments = power_domain_arm_core_fragments,
    .fragments_count = countof(power_domain_arm_core_fragments),
    .coresident_device_index = 0,
    .metadata_list = power_domain_big_core,
    .metadata_count = countof(power_domain_big_core),
};

constexpr power_domain_t little_domain[] = {
    {static_cast<uint32_t>(T931PowerDomains::kArmCoreLittle)},
};

constexpr device_metadata_t power_domain_little_core[] = {
    {
        .type = DEVICE_METADATA_POWER_DOMAINS,
        .data = &little_domain,
        .length = sizeof(little_domain),
    },
};

constexpr composite_device_desc_t power_domain_little_core_desc = {
    .props = power_domain_arm_core_props,
    .props_count = countof(power_domain_arm_core_props),
    .fragments = power_domain_arm_core_fragments,
    .fragments_count = countof(power_domain_arm_core_fragments),
    .coresident_device_index = 0,
    .metadata_list = power_domain_little_core,
    .metadata_count = countof(power_domain_little_core),
};

constexpr zx_bind_inst_t pwm_ao_d_match[] = {
    BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_PWM),
    BI_MATCH_IF(EQ, BIND_PWM_ID, T931_PWM_AO_D),
};

constexpr zx_bind_inst_t pwm_a_match[] = {
    BI_ABORT_IF(NE, BIND_PROTOCOL, ZX_PROTOCOL_PWM),
    BI_MATCH_IF(EQ, BIND_PWM_ID, T931_PWM_A),
};

constexpr device_fragment_part_t pwm_ao_d_fragment[] = {
    {countof(root_match), root_match},
    {countof(pwm_ao_d_match), pwm_ao_d_match},
};

constexpr device_fragment_part_t pwm_a_fragment[] = {
    {countof(root_match), root_match},
    {countof(pwm_a_match), pwm_a_match},
};

constexpr device_fragment_t power_impl_fragments[] = {
    {countof(pwm_ao_d_fragment), pwm_ao_d_fragment},
    {countof(pwm_a_fragment), pwm_a_fragment},
};

}  // namespace

static const pbus_dev_t power_dev = []() {
  pbus_dev_t dev = {};
  dev.name = "aml-power-impl-composite";
  dev.vid = PDEV_VID_AMLOGIC;
  dev.pid = PDEV_PID_AMLOGIC_T931;
  dev.did = PDEV_DID_AMLOGIC_POWER;
  dev.metadata_list = power_impl_metadata;
  dev.metadata_count = countof(power_impl_metadata);
  return dev;
}();

zx_status_t Sherlock::PowerInit() {
  zx_status_t st;

  // Configure the GPIO to be Output & set it to alternate
  // function 3 which puts in PWM_D mode. A53 cluster (Small)
  gpio_impl_.SetAltFunction(T931_GPIOE(1), aml_pwm::kPwmDFn);

  st = gpio_impl_.ConfigOut(T931_GPIOE(1), 0);
  if (st != ZX_OK) {
    zxlogf(ERROR, "%s: ConfigOut failed: %d", __func__, st);
    return st;
  }

  // Configure the GPIO to be Output & set it to alternate
  // function 3 which puts in PWM_D mode. A73 cluster (Big)
  gpio_impl_.SetAltFunction(T931_GPIOE(2), aml_pwm::kPwmDFn);

  st = gpio_impl_.ConfigOut(T931_GPIOE(2), 0);
  if (st != ZX_OK) {
    zxlogf(ERROR, "%s: ConfigOut failed: %d", __func__, st);
    return st;
  }

  st = pbus_.CompositeDeviceAdd(&power_dev, power_impl_fragments, countof(power_impl_fragments),
                                UINT32_MAX);
  if (st != ZX_OK) {
    zxlogf(ERROR, "%s: CompositeDeviceAdd for powerimpl failed, st = %d", __FUNCTION__, st);
    return st;
  }

  st = DdkAddComposite("composite-pd-big-core", &power_domain_big_core_desc);
  if (st != ZX_OK) {
    zxlogf(ERROR, "%s: CompositeDeviceAdd for power domain Big Arm Core failed, st = %d", __FUNCTION__,
           st);
    return st;
  }

st = DdkAddComposite("composite-pd-little-core", &power_domain_little_core_desc);
  if (st != ZX_OK) {
    zxlogf(ERROR, "%s: CompositeDeviceAdd for power domain Little Arm Core failed, st = %d", __FUNCTION__,
           st);
    return st;
  }

  return ZX_OK;
}

}  // namespace sherlock
