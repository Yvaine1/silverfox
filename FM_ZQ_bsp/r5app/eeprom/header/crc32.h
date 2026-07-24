/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */

#ifndef _CRC32_H_
#define _CRC32_H_

#include "dg_common.h"

#define CRC32(crc,ch)       (crc_32Tab[((INT32)(crc ^ (ch))) & 0xff] ^ (crc >> 8))
#define CRC_SEED 0xffffffffL

#ifdef __cplusplus
extern "C" {
#endif

    UINT32 crc32buf ( UINT32, CHAR *, INT32 );

#ifdef __cplusplus
}
#endif


#endif
