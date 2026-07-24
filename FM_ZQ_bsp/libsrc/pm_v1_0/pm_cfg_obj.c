/******************************************************************************
*
* Copyright (C) 2017 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/

#include "fmsh_common_types.h"
#include "pm_defs.h"

#define PM_CONFIG_MASTER_SECTION_ID	0x101U
#define PM_CONFIG_SLAVE_SECTION_ID	0x102U
#define PM_CONFIG_PREALLOC_SECTION_ID	0x103U
#define PM_CONFIG_POWER_SECTION_ID	0x104U
#define PM_CONFIG_RESET_SECTION_ID	0x105U
#define PM_CONFIG_SHUTDOWN_SECTION_ID	0x106U
#define PM_CONFIG_SET_CONFIG_SECTION_ID	0x107U
#define PM_CONFIG_GPO_SECTION_ID	0x108U

/* Type of Config Obejcts */
#define PM_CONFIG_OBJECT_TYPE_BASE	(1U)
#define PM_CONFIG_OBJECT_TYPE_OVERLAY	(2U)

#define PM_SLAVE_FLAG_IS_SHAREABLE	0x1U
#define PM_MASTER_USING_SLAVE_MASK	0x2U

#define PM_CONFIG_GPO1_MIO_PIN_34_MAP	(1U << 10U)
#define PM_CONFIG_GPO1_MIO_PIN_35_MAP	(1U << 11U)
#define PM_CONFIG_GPO1_MIO_PIN_36_MAP	(1U << 12U)
#define PM_CONFIG_GPO1_MIO_PIN_37_MAP	(1U << 13U)

#define PM_CONFIG_GPO1_BIT_2_MASK	(1U << 2U)
#define PM_CONFIG_GPO1_BIT_3_MASK	(1U << 3U)
#define PM_CONFIG_GPO1_BIT_4_MASK	(1U << 4U)
#define PM_CONFIG_GPO1_BIT_5_MASK	(1U << 5U)

#define SUSPEND_TIMEOUT	0xFFFFFFFFU


#define PM_CONFIG_IPI_PSU_CORTEXA53_0_MASK    0x00000001
#define PM_CONFIG_IPI_PSU_CORTEXR5_0_MASK    0x00000100
#define PM_CONFIG_IPI_PSU_CORTEXR5_1_MASK    0x00000200



#if defined (__ICCARM__)
#pragma language=save
#pragma language=extended
#endif
#if defined (__GNUC__)
    const u32 FPm_ConfigObject[] __attribute__((used, section(".sys_cfg_data"))) =
#elif defined (__ICCARM__)
#pragma location = ".sys_cfg_data"
__root const u32 FPm_ConfigObject[] =
#endif
{
	/**********************************************************************/
	/* HEADER */
	2,	/* Number of remaining words in the header */
    1,	/* Number of sections in configuration object */
    PM_CONFIG_OBJECT_TYPE_OVERLAY,	/* Type of Config Obejcts */
	/**********************************************************************/
	/* PREALLOC SECTION */
	PM_CONFIG_PREALLOC_SECTION_ID,	/* Section ID */
	2,				/* Number of masters */
					/* Master #1 */
	PM_CONFIG_IPI_PSU_CORTEXA53_0_MASK,		/* IPI mask of the master */
	9,				/* Number of preallocated slaves */
					/* Slave #1 */
	NODE_IPI_APU,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS,			/* Current requirements */
	PM_CAP_ACCESS,			/* Default requirements */
					/* Slave #2 */
	NODE_DDR,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
					/* Slave #3 */
	NODE_L2,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Default requirements */
					/* Slave #4 */
	NODE_OCM_BANK_0,		/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
					/* Slave #5 */
	NODE_OCM_BANK_1,		/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
					/* Slave #6 */
	NODE_OCM_BANK_2,		/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
					/* Slave #7 */
	NODE_OCM_BANK_3,		/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
					/* Slave #8 */
	NODE_GPU,		/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
					/* Slave #9 */
	NODE_VPU,		/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */

	PM_CONFIG_IPI_PSU_CORTEXR5_0_MASK,	/* IPI mask of the master */
	6,				/* Number of preallocated slaves */
					/* Slave #1 */
	NODE_IPI_RPU_0,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS,			/* Current requirements */
	PM_CAP_ACCESS,			/* Default requirements */
					/* Slave #2 */
	NODE_TCM_0_A,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Default requirements */
					/* Slave #3 */
	NODE_TCM_0_B,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Default requirements */
					/* Slave #4 */
	NODE_TCM_1_A,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Default requirements */
					/* Slave #5 */
	NODE_TCM_1_B,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Default requirements */
					/* Slave #6 */
	NODE_DDR,			/* Node ID of the slave */
	PM_MASTER_USING_SLAVE_MASK,	/* Flag (is the slave currently used) */
	PM_CAP_ACCESS | PM_CAP_CONTEXT,	/* Current requirements */
	0,				/* Default requirements */
};
#if defined (__ICCARM__)
#pragma language=restore
#endif

