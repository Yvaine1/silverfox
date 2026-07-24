#ifndef _CRC32_H
#define _CRC32_H

#include "fmsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

u32 calculate_crc32(const uint8_t *data, u32 length);

#ifdef __cplusplus
}
#endif


#endif