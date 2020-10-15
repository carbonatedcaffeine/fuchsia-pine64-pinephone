// Copyright 2019 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#define HAS_DEVICE_TREE 1

static const zbi_mem_range_t mem_config[] = {
    // The bootloader adds this to the device tree.
    // {
    //     .paddr = 0x40000000,
    //     .length = 0x40000000,  // 1G
    //     .type = ZBI_MEM_RANGE_RAM,
    // },
    {
        .paddr = 0x00000000,
        .length = 0x40000000,
        .type = ZBI_MEM_RANGE_PERIPHERAL,
    },
};

static const dcfg_simple_t uart_driver = {
    .mmio_phys = 0x30860000,
    .irq = 58,
};

static const dcfg_arm_gicv3_driver_t gicv3_driver = {
    .mmio_phys = 0x38800000,
    .gicd_offset = 0x00000,
    .gicr_offset = 0x80000,
    .gicr_stride = 0x20000,
    .mx8_gpr_phys = 0x30340000,
    .ipi_base = 0,
};

static const dcfg_arm_psci_driver_t psci_driver = {
    .use_hvc = false,
};

static const dcfg_arm_generic_timer_driver_t timer_driver = {
    .irq_phys = 30, .irq_virt = 27,
    .freq_override = 8333333,
};

static const zbi_platform_id_t platform_id = {
    PDEV_VID_NXP,
    PDEV_PID_IMX8M,
    "imx8m",
};

static void add_cpu_topology(zbi_header_t* zbi) {
// TODO(bradenkell): Enable secondaries
// #define TOPOLOGY_CPU_COUNT 4
#define TOPOLOGY_CPU_COUNT 4
  zbi_topology_node_t nodes[TOPOLOGY_CPU_COUNT];

  for (uint8_t index = 0; index < TOPOLOGY_CPU_COUNT; index++) {
    nodes[index] = (zbi_topology_node_t){
        .entity_type = ZBI_TOPOLOGY_ENTITY_PROCESSOR,
        .parent_index = ZBI_TOPOLOGY_NO_PARENT,
        .entity =
            {
                .processor =
                    {
                        .logical_ids = {index},
                        .logical_id_count = 1,
                        .flags = (uint16_t)(index == 0 ? ZBI_TOPOLOGY_PROCESSOR_PRIMARY : 0),
                        .architecture = ZBI_TOPOLOGY_ARCH_ARM,
                        .architecture_info =
                            {
                                .arm =
                                    {
                                        .cpu_id = index,
                                        .gic_id = index,
                                    },
                            },
                    },
            },
    };
  }

  append_boot_item(zbi, ZBI_TYPE_CPU_TOPOLOGY, sizeof(zbi_topology_node_t), &nodes,
                   sizeof(zbi_topology_node_t) * TOPOLOGY_CPU_COUNT);
}

static void append_board_boot_item(zbi_header_t* bootdata) {
  add_cpu_topology(bootdata);

  // add memory configuration
  append_boot_item(bootdata, ZBI_TYPE_MEM_CONFIG, 0, &mem_config,
                   sizeof(zbi_mem_range_t) * countof(mem_config));

  // add kernel drivers
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_NXP_IMX_UART, &uart_driver,
                   sizeof(uart_driver));
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GIC_V3, &gicv3_driver,
                   sizeof(gicv3_driver));
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_PSCI, &psci_driver,
                   sizeof(psci_driver));
  append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GENERIC_TIMER, &timer_driver,
                   sizeof(timer_driver));

  // add platform ID
  append_boot_item(bootdata, ZBI_TYPE_PLATFORM_ID, 0, &platform_id, sizeof(platform_id));
}
