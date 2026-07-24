/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sdmmc_example.h
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   yl  12/20/2018  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_SDMMC_EXAMPLE_H_
#define _FMSH_SDMMC_EXAMPLE_H_

#include "Fatfs15\ff.h"
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSdPsu_example(u16 deviceId);
int FSdPsu_fs_multi_partitions_example();
void show_all_file_info_of_dir (TCHAR *path);
void show_all_dir_of_partition (TCHAR *path);
void remove_all_dirs (TCHAR *path);
FRESULT remove_file (TCHAR *path);
u32 fmsh_SdEmmcInitPartFAT32 (u32 ulPhyDriveNo, u32 ulPartitionNum,
                                     DWORD plist[]);
void fdisk_physicaldrive (u32 ulPhyDriveNo);

#endif /* _FMSH_SDMMC_EXAMPLE_H_ */
