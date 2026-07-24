/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2023 FMSH, Inc.
 */

#ifndef _RELEASE_RPU_H
#define _RELEASE_RPU_H

#define FMZQ_TCM_BASE_ADDR		0xFFE00000
#define FMZQ_TCM_SIZE			0x40000

#define FMZQ_CRL_APB_BASEADDR	0xFF5E0000
#define FMZQ_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT	0x1000000
#define FMZQ_CRL_APB_BOOT_PIN_CTRL_OUT_EN_SHIFT	0
#define FMZQ_CRL_APB_BOOT_PIN_CTRL_OUT_VAL_SHIFT	8

#define PS_MODE0	BIT(0)
#define PS_MODE1	BIT(1)
#define PS_MODE2	BIT(2)
#define PS_MODE3	BIT(3)

#define RESET_REASON_DEBUG_SYS	BIT(6)
#define RESET_REASON_SOFT	BIT(5)
#define RESET_REASON_SRST	BIT(4)
#define RESET_REASON_PSONLY	BIT(3)
#define RESET_REASON_PMU	BIT(2)
#define RESET_REASON_INTERNAL	BIT(1)
#define RESET_REASON_EXTERNAL	BIT(0)

#define PM_MMIO_WRITE 19


#define LOCK		0
#define SPLIT		1

#define HALT		0
#define RELEASE		1

#define FMZQ_R5_HIVEC_ADDR			0xFFFF0000
#define FMZQ_R5_LOVEC_ADDR			0x0
#define FMZQ_RPU_CFG_CPU_HALT_MASK		0x01
#define FMZQ_RPU_CFG_HIVEC_MASK		0x04
#define FMZQ_RPU_GLBL_CTRL_SPLIT_LOCK_MASK	0x08
#define FMZQ_RPU_GLBL_CTRL_TCM_COMB_MASK	0x40
#define FMZQ_RPU_GLBL_CTRL_SLCLAMP_MASK	0x10

#define FMZQ_CRLAPB_RST_LPD_R50_RST_MASK	0x01
#define FMZQ_CRLAPB_RST_LPD_R51_RST_MASK	0x02
#define FMZQ_CRLAPB_CPU_R5_CTRL_CLKACT_MASK	0x1000000

#define FMZQ_R5_0_TCM_START_ADDR		0xFFE00000
#define FMZQ_R5_1_TCM_START_ADDR		0xFFE90000

#define FMZQ_R5_0_BTCM_START_ADDR		0xFFE20000
#define FMZQ_R5_1_BTCM_START_ADDR		0xFFEB0000

#define RPU_TCM_SIZE         0x10000

#define FMZQ_CORE_RPU0	4
#define FMZQ_CORE_RPU1	5
#define FMZQ_MAX_CORES	6

#define FMZQ_RPU0_USE_MASK BIT(1)
#define FMZQ_RPU1_USE_MASK BIT(2)


struct crlapb_regs {
	u32 reserved0[36];
	u32 cpu_r5_ctrl; /* 0x90 */
	u32 reserved1[37];
	u32 timestamp_ref_ctrl; /* 0x128 */
	u32 reserved2[53];
	u32 boot_mode; /* 0x200 */
	u32 reserved3_0[7];
	u32 reset_reason; /* 0x220 */
	u32 reserved3_1[6];
	u32 rst_lpd_top; /* 0x23C */
	u32 reserved4[4];
	u32 boot_pin_ctrl; /* 0x250 */
	u32 reserved5[21];
};

#define crlapb_base ((struct crlapb_regs *)FMZQ_CRL_APB_BASEADDR)

#define FMZQ_IOU_SCNTR_SECURE	0xFF260000
#define FMZQ_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN	0x1
#define FMZQ_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG	0x2

struct iou_scntr_secure {
	u32 counter_control_register;
	u32 reserved0[7];
	u32 base_frequency_id_register;
};

#define iou_scntr_secure ((struct iou_scntr_secure *)FMZQ_IOU_SCNTR_SECURE)

/* Bootmode setting values */
// #define BOOT_MODES_MASK	0x0000000F
#define QSPI_MODE_24BIT	0x00000001
#define QSPI_MODE_32BIT	0x00000002
#define SD_MODE		0x00000003 /* sd 0 */
#define SD_MODE1	0x00000005 /* sd 1 */
#define NAND_MODE	0x00000004
#define EMMC_MODE	0x00000006
#define USB_MODE	0x00000007
#define SD1_LSHFT_MODE	0x0000000E /* SD1 Level shifter */
#define JTAG_MODE	0x00000000
#define BOOT_MODE_USE_ALT	0x100
#define BOOT_MODE_ALT_SHIFT	12
/* SW secondary boot modes 0xa - 0xd */
#define SW_USBHOST_MODE	0x0000000A
#define SW_SATA_MODE	0x0000000B

#define FMZQ_IOU_SLCR_BASEADDR	0xFF180000

struct iou_slcr_regs {
	u32 mio_pin[78];
	u32 reserved[442];
};


#define FMZQ_RPU_BASEADDR	0xFF9A0000

struct rpu_regs {
	u32 rpu_glbl_ctrl;
	u32 reserved0[63];
	u32 rpu0_cfg; /* 0x100 */
	u32 reserved1[63];
	u32 rpu1_cfg; /* 0x200 */
};

#define rpu_base ((struct rpu_regs *)FMZQ_RPU_BASEADDR)

#define FMZQ_CRF_APB_BASEADDR	0xFD1A0000

struct crfapb_regs {
	u32 reserved0[65];
	u32 rst_fpd_apu; /* 0x104 */
	u32 reserved1;
};

#define crfapb_base ((struct crfapb_regs *)FMZQ_CRF_APB_BASEADDR)

#define FMZQ_APU_BASEADDR	0xFD5C0000

struct apu_regs {
	u32 reserved0[16];
	u32 rvbar_addr0_l; /* 0x40 */
	u32 rvbar_addr0_h; /* 0x44 */
	u32 reserved1[20];
};

#define apu_base ((struct apu_regs *)FMZQ_APU_BASEADDR)

/* Board version value */
#define FMZQ_CSU_BASEADDR		0xFFCA0000
#define FMZQ_CSU_VERSION_SILICON	0x0
#define FMZQ_CSU_VERSION_QEMU		0x3

#define FMZQ_CSU_VERSION_EMPTY_SHIFT		20

#define FMZQ_SILICON_VER_MASK		0xF000
#define FMZQ_SILICON_VER_SHIFT	12

struct csu_regs {
	u32 reserved0[4];
	u32 multi_boot;
	u32 reserved1[12];
	u32 version;
};

#define csu_base ((struct csu_regs *)FMZQ_CSU_BASEADDR)
#define FMZQ_CSU_MULTIBOOT_ADDR	0xE0040038

#define FMZQ_PMU_BASEADDR	0xFFD80000


struct pmu_regs {
	u32 reserved0[16];
	u32 gen_storage4; /* 0x40 */
	u32 reserved1[1];
	u32 gen_storage6; /* 0x48 */
};

#define pmu_base ((struct pmu_regs *)FMZQ_PMU_BASEADDR)

#define FMZQ_CSU_IDCODE_ADDR	0xE0040720
#define FMZQ_CSU_VER_ADDR		0xE0040724


int cpu_release(u32 nr, u32 boot_addr);
void initialize_rpu_tcm(u32 nr);

#endif /* _ASM_ARCH_HARDWARE_H */
