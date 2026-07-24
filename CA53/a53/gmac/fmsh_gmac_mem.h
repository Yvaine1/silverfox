
#ifndef _FMSH_GMAC_MEM_H
#define _FMSH_GMAC_MEM_H

#include "fmsh_common_types.h"

struct frame_memp {
    struct frame_memp *next;
};

struct frame_desc{
    const char *desc;
    u16 size;
    u16 num;
    u8 *base;
    struct frame_memp **tap;
};

void frame_mem_init(struct frame_desc *mac_desc);
void * frame_mem_malloc(struct frame_desc *mac_desc);
void frame_mem_free(struct frame_desc *mac_desc,void *mem);
void gmac_0_prv_test(void *pvParameters);
void gmac_0_prv_tx(void *pvParameters);

#endif
