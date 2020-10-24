// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

static const zbi_cpu_config_t cpu_config = {
    .cluster_count = 1,
    .clusters =
        {
            {
                .cpu_count = 2,
            },
        },
};

static const zbi_mem_range_t mem_config[] = {
    {
        .type = ZBI_MEM_RANGE_RAM,
        .paddr = 0x40000000,
        .length = 0x0x40000000, // 1GB
    },
    {
        .type = ZBI_MEM_RANGE_PERIPHERAL,
        // This is not the entire peripheral range, but enough to cover what we use in the kernel.
        .paddr = 0x0c000000,
        .length = 0x34000000,
    },
};

static const dcfg_simple_t uart_driver = {
    .mmio_phys = 0x01C28000,
    .irq = 33,
};

static const dcfg_arm_gicv2_driver_t gicv2_driver = {
    .mmio_phys = 0x01c81000,
    .gicd_offset = 0x1000,
    .gicc_offset = 0x2000,
    .ipi_base = 9,
};

static const dcfg_arm_psci_driver_t psci_driver = {
    .use_hvc = false,
};

static const dcfg_arm_generic_timer_driver_t timer_driver = {
    .irq_phys = 16 + 14,  // PHYS_NONSECURE_PPI: GIC_PPI 14
    .irq_virt = 16 + 11,  // VIRT_PPI: GIC_PPI 11
};

static void append_board_boot_item(zbi_header_t* bootdata) {
  // Disable watchdog timer for now.
  // TODO - remove this after we have a proper watchdog driver in userspace.
  disable_watchdog();

  // add CPU configuration
  append_boot_item(bootdata, ZBI_TYPE_CPU_CONFIG, 0, &cpu_config,
                   sizeof(zbi_cpu_config_t) + sizeof(zbi_cpu_cluster_t) * cpu_config.cluster_count);

  // add memory configuration
  append_boot_item(bootdata, ZBI_TYPE_MEM_CONFIG, 0, &mem_config,
                   sizeof(zbi_mem_range_t) * countof(mem_config));

  // add kernel drivers
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_MT8167_UART, &uart_driver,
                   sizeof(uart_driver));
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GIC_V2, &gicv2_driver,
                   sizeof(gicv2_driver));
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_PSCI, &psci_driver,
                   sizeof(psci_driver));
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GENERIC_TIMER, &timer_driver,
                   sizeof(timer_driver));
