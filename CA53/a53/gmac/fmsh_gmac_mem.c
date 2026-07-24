
#include "fmsh_gmac_interface.h"
#include "fmsh_gmac_mem.h"
#include "lwip/arch.h"
#include "sys_arch.h"
#include "sys.h"

#define FRAME_SIZE          LWIP_MEM_ALIGN_SIZE(0x680)
#define RXFRAME_CNT         4096 /* Number of buffer to use */
#define FRAME_ALIGN_SIZE    64

//__attribute__ ((section(".rxframe"), aligned (FRAME_ALIGN_SIZE)))
__no_init u8 RxFrame_rxtx0[RXFRAME_CNT * FRAME_SIZE] __attribute__ ((aligned (FRAME_ALIGN_SIZE)));/*  Receive buffer */

__attribute__ ((section(".rxframe"), aligned (FRAME_ALIGN_SIZE)))
__no_init EthernetFrame RxFrame_rxtx1[1] __attribute__ ((aligned (FRAME_ALIGN_SIZE)));/*  Receive buffer */

//__attribute__ ((section(".rxframe"), aligned (FRAME_ALIGN_SIZE)))
__no_init u8 RxFrame_rxtx2[RXFRAME_CNT * FRAME_SIZE] __attribute__ ((aligned (FRAME_ALIGN_SIZE)));/*  Receive buffer */

__attribute__ ((section(".rxframe"), aligned (FRAME_ALIGN_SIZE)))
__no_init EthernetFrame RxFrame_rxtx3[1] __attribute__ ((aligned (FRAME_ALIGN_SIZE)));/*  Receive buffer */

u32 frame_use_count = 0;
int frame_test = 0;


__attribute__ ((section(".rxframe")))
struct frame_desc gmac_desc_0 = {
    .desc = "gmac0",
    .size = FRAME_SIZE,
    .num  = RXFRAME_CNT,
    .base = RxFrame_rxtx0,
    .tap  = NULL,
};

__attribute__ ((section(".rxframe")))
struct frame_desc gmac_desc_2 = {
    .desc = "gmac2",
    .size = FRAME_SIZE,
    .num  = RXFRAME_CNT,
    .base = RxFrame_rxtx2,
    .tap  = NULL,
};

void frame_mem_init(struct frame_desc *mac_desc)
{
    int i;
    struct frame_memp *frame_mem;

    *mac_desc->tap = NULL;
    frame_mem = (struct frame_memp *)(mac_desc->base);

    memset(frame_mem,0x0,(size_t)mac_desc->num*(mac_desc->size));

    for (i = 0; i < mac_desc->num;i++)
    {
        frame_mem->next = *mac_desc->tap;
        *mac_desc->tap = frame_mem;
        frame_mem = (struct frame_memp*)(void*)((u8 *)frame_mem + mac_desc->size);
    }
}

void * frame_mem_malloc(struct frame_desc *mac_desc)
{
    struct frame_memp *frame_mem;
    
    unsigned long cur;
    frame_mem = *mac_desc->tap;

    if ((frame_mem != NULL) && (frame_mem < (struct frame_memp *)0x80000000))
    {
        *mac_desc->tap = frame_mem->next;
        return ((u8 *)frame_mem);
    }

    return NULL;
}

void frame_mem_free(struct frame_desc *mac_desc,void *mem)
{
    unsigned long cur;
    struct frame_memp *frame_mem;
    frame_mem = mem;

    frame_mem->next = *mac_desc->tap;
    *mac_desc->tap = frame_mem;
}
