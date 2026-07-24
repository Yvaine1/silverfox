/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_axidmapsu_g.c
 * @addtogroup axidmapsu_v1_0
 * @{
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---- --------   ---------------------------------------------
 * 1.00  whn 07/18/2024  First Release
 *
 * </pre>
 *
 ******************************************************************************/
#include "fmsh_parameters.h"
#include "fmsh_axidmapsu.h"

/******************************************************************************
 * This table contains configuration information for each AXI DMA
 * device in the system.
 ******************************************************************************/
FAxidmaPsu_Config_T FAxidmaPsu_ConfigTable[] = {
    {
        FPAR_AXIDMAPSU_0_DEVICE_ID,
        FPAR_AXIDMAPSU_NUM_CHANNEL, 
        FPAR_AXIDMAPSU_0_BASEADDR,
    },
    {
        FPAR_AXIDMAPSU_1_DEVICE_ID,
        FPAR_AXIDMAPSU_NUM_CHANNEL,
        FPAR_AXIDMAPSU_1_BASEADDR,
    }
      
};
