 /*
 * Copyright (c) 2024, FMSH Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fmsh_common.h"
#include "fmsh_print.h"
#include "fmsh_psu_parameters.h"
#include "psu_init.h"
#include "fmsh_common_types.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"

#include "platform_config.h"
#include "platform.h"
#include "common.h"

#include <metal/io.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <metal/irq.h>

/* Default generic I/O region page shift */
/* Each I/O region can contain multiple pages.
 * In baremetal system, the memory mapping is flat, there is no
 * virtual memory.
 * We can assume there is only one page in the whole baremetal system.
 */
#define DEFAULT_PAGE_SHIFT (-1UL)
#define DEFAULT_PAGE_MASK  (-1UL)

const metal_phys_addr_t metal_phys[] = {
	IPI_BASE_ADDR, /**< base IPI address */
	SHM_BASE_ADDR, /**< shared memory base address */
	TTC0_BASE_ADDR, /**< base TTC0 address */
};

/* Define metal devices table for IPI, shared memory and TTC devices.
 * Linux system uses device tree to describe devices. Unlike Linux,
 * there is no standard device abstraction for baremetal system, we
 * uses libmetal devices structure to describe the devices we used in
 * the example.
 * The IPI, shared memory and TTC devices are memory mapped
 * devices. For this type of devices, it is required to provide
 * accessible memory mapped regions, and interrupt information.
 * In baremetal system, the memory mapping is flat. As you can see
 * in the table before, we set the virtual address "virt" the same
 * as the physical address.
 */
static struct metal_device metal_dev_table[] = {
	{
		/* IPI device */
		.name = IPI_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = {
			{
				.virt = (void *)IPI_BASE_ADDR,
				.physmap = &metal_phys[0],
				.size = 0x1000,
				.page_shift = DEFAULT_PAGE_SHIFT,
				.page_mask = DEFAULT_PAGE_MASK,
				.mem_flags = DEVICE_NONSHARED | PRIV_RW_USER_RW,
				.ops = {NULL},
			}
		},
		.node = {NULL},
		.irq_num = 1,
		.irq_info = (void *)IPI_IRQ_VECT_ID,
	},
	{
		/* Shared memory management device */
		.name = SHM_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = {
			{
				.virt = (void *)SHM_BASE_ADDR,
				.physmap = &metal_phys[1],
				.size = 0x1000000,
				.page_shift = DEFAULT_PAGE_SHIFT,
				.page_mask = DEFAULT_PAGE_MASK,
				.mem_flags = NORM_SHARED_NCACHE |
						PRIV_RW_USER_RW,
				.ops = {NULL},
			}
		},
		.node = {NULL},
		.irq_num = 0,
		.irq_info = NULL,
	},
	{
		/* ttc0 */
		.name = TTC_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = {
			{
				.virt = (void *)TTC0_BASE_ADDR ,
				.physmap = &metal_phys[2],
				.size = 0x1000,
				.page_shift = DEFAULT_PAGE_SHIFT,
				.page_mask = DEFAULT_PAGE_MASK,
				.mem_flags = DEVICE_NONSHARED | PRIV_RW_USER_RW,
				.ops = {NULL},
			}
		},
		.node = {NULL},
		.irq_num = 0,
		.irq_info = NULL,
	},
};

/**
 * Extern global variables
 */
struct metal_device *ipi_dev = NULL;
struct metal_device *shm_dev = NULL;
struct metal_device *ttc_dev = NULL;

extern u32 FGicPs_CommonInit (FGicPs *InstancePtr);
extern u32 FGicPs_SelfTest (FGicPs *InstancePtr);
/**
 * @brief init_irq() - Initialize GIC and connect IPI interrupt
 *        This function will initialize the GIC and connect the IPI
 *        interrupt.
 *
 * @return 0 - succeeded, non-0 for failures
 */
int init_irq()
{
	int ret = 0;

    ret = FGicPs_CommonInit(&IntcInstance);
    if (ret != GIC_SUCCESS)
    {
        fmsh_print("GIC Setup Failed!\r\n");
    }
    else
    {
        fmsh_print("GIC Setup pass!\r\n");
    }

    ret = FGicPs_SelfTest(&IntcInstance);
    if (ret != GIC_SUCCESS)
    {
        fmsh_print("GIC Selftest Failed!\r\n");
    }
    else
    {
        fmsh_print("GIC Selftest pass!\r\n");
    }
	
	/* Connect IPI Interrupt ID with libmetal ISR */
    ret = FGicPs_Connect(&IntcInstance, IPI_IRQ_VECT_ID,
                            (FMSH_InterruptHandler)metal_fmsh_irq_isr, (void *)IPI_IRQ_VECT_ID);
    if (ret != GIC_SUCCESS)
    {
        return GIC_FAILURE;
    }

    FGicPs_Enable(&IntcInstance, IPI_IRQ_VECT_ID);

	return 0;
}

/**
 * @brief platform_register_metal_device() - Statically Register libmetal
 *        devices.
 *        This function registers the IPI, shared memory and
 *        TTC devices to the libmetal generic bus.
 *        Libmetal uses bus structure to group the devices. Before you can
 *        access the device with libmetal device operation, you will need to
 *        register the device to a libmetal supported bus.
 *        For non-Linux system, libmetal only supports "generic" bus, which is
 *        used to manage the memory mapped devices.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int platform_register_metal_device(void)
{
	unsigned int i;
	int ret;
	struct metal_device *dev;

	for (i = 0; i < sizeof(metal_dev_table)/sizeof(struct metal_device);
	     i++) {
		dev = &metal_dev_table[i];
		fmsh_print("registering: %d, name=%s\n", i, dev->name);
		ret = metal_register_generic_device(dev);
		if (ret)
			return ret;
	}
	return 0;
}

/**
 * @brief open_metal_devices() - Open registered libmetal devices.
 *        This function opens all the registered libmetal devices.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int open_metal_devices(void)
{
	int ret;

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &ttc_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}

out:
	return ret;
}

/**
 * @brief close_metal_devices() - close libmetal devices
 *        This function closes all the libmetal devices which have
 *        been opened.
 *
 */
void close_metal_devices(void)
{
	/* Close shared memory device */
	if (shm_dev)
		metal_device_close(shm_dev);

	/* Close IPI device */
	if (ipi_dev)
		metal_device_close(ipi_dev);

	/* Close TTC device */
	if (ttc_dev)
		metal_device_close(ttc_dev);
}

static const struct {
    uint64_t size;
    uint32_t encoding;
}region_size[] = {
    { 0x20, REGION_32B },
    { 0x40, REGION_64B },
    { 0x80, REGION_128B },
    { 0x100, REGION_256B },
    { 0x200, REGION_512B },
    { 0x400, REGION_1K },
    { 0x800, REGION_2K },
    { 0x1000, REGION_4K },
    { 0x2000, REGION_8K },
    { 0x4000, REGION_16K },
    { 0x8000, REGION_32K },
    { 0x10000, REGION_64K },
    { 0x20000, REGION_128K },
    { 0x40000, REGION_256K },
    { 0x80000, REGION_512K },
    { 0x100000, REGION_1M },
    { 0x200000, REGION_2M },
    { 0x400000, REGION_4M },
    { 0x800000, REGION_8M },
    { 0x1000000, REGION_16M },
    { 0x2000000, REGION_32M },
    { 0x4000000, REGION_64M },
    { 0x8000000, REGION_128M },
    { 0x10000000, REGION_256M },
    { 0x20000000, REGION_512M },
    { 0x40000000, REGION_1G },
    { 0x80000000, REGION_2G },
    { 0x100000000, REGION_4G },
};
/**
uint32_t SetMPURegion(uint32_t addr, uint64_t size, int region_num,
                        uint32_t attrib)
{
	uint32_t local_addr = addr;
	uint32_t region_val;
	int i;

	local_addr &= (uint32_t)(~(size - 1U)); 
	size += addr - local_addr;
	
 
    for (i = 0; i < (sizeof (region_size) / sizeof (region_size[0])); i++) {
        if (size <= region_size[i].size) {
            region_val = region_size[i].encoding;
            break;
        }
    }

    Fmsh_SetAttribute(addr, region_val, region_num, attrib);
	
	return 0;
}
**/
/**
 * @brief sys_init() - Register libmetal devices.
 *        This function register the libmetal generic bus, and then
 *        register the IPI, shared memory descriptor and shared memory
 *        devices to the libmetal generic bus.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int sys_init()
{
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;
	int ret;
	int i;

	init_platform();
	
	if (init_irq()) {
		LPERROR("Failed to initialize interrupt\n");
	}
	
	/* Initialize libmetal environment */
	metal_init(&metal_param);
	/* Initialize metal FMSH IRQ controller */
	ret = metal_fmsh_irq_init();
	if (ret) {
		LPERROR("%s: FMSH metal IRQ controller init failed.\n",
			__func__);
		return ret;
	}
	
	/* mmap device regions */
	SetMPURegion((uint32_t)metal_dev_table[0].regions[0].virt,
					  metal_dev_table[0].regions[0].size,
					  7, metal_dev_table[0].regions[0].mem_flags);
	SetMPURegion((uint32_t)metal_dev_table[1].regions[0].virt,
					  metal_dev_table[1].regions[0].size,
					  8, metal_dev_table[1].regions[0].mem_flags);
	SetMPURegion((uint32_t)metal_dev_table[2].regions[0].virt,
					  metal_dev_table[2].regions[0].size,
					  9, metal_dev_table[2].regions[0].mem_flags);

	
	/* Register libmetal devices */
	ret = platform_register_metal_device();
	if (ret) {
		LPERROR("%s: failed to register devices: %d\n", __func__, ret);
		return ret;
	}

	/* Open libmetal devices which have been registered */
	ret = open_metal_devices();
	if (ret) {
		LPERROR("%s: failed to open devices: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

/**
 * @brief sys_cleanup() - system cleanup
 *        This function finish the libmetal environment
 *        and disable caches.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
void sys_cleanup()
{
	/* Close libmetal devices which have been opened */
	close_metal_devices();
	/* Finish libmetal environment */
	metal_finish();
	cleanup_platform();
}

