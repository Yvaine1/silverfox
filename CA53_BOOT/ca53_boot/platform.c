/*
 * Uncomment one of the following two lines, depending on the target,
 * if ps7/psu init source files are added in the source directory for
 * compiling example outside of IAR.
 */
#include "fmsh_common.h"
#include "fmsh_print.h"
#include "fmsh_psu_parameters.h"
#include "psu_init.h"
#include "fmsh_gic.h"


extern u32 FGicPs_CommonInit (FGicPs *InstancePtr);

void init_platform ()
{
    int ret = PSU_INIT_SUCCESS;

#if PS_PREINITED == 0
    ret = psu_init();
#if USE_DDR == 1
   #if (DDR_SIZE >= 0x80000000)
    Fmsh_SetTlbAttributesRange(0x0, 0x7f000000, ATTR_MEM);
    Fmsh_SetTlbAttributesRange(0x7f000000, 0x1000000, ATTR_MEM_NC);
#else  /*DDR_SIZE*/
    Fmsh_SetTlbAttributesRange(0x0, DDR_SIZE - 0x1000000, ATTR_MEM);
    Fmsh_SetTlbAttributesRange(DDR_SIZE - 0x1000000, 0x1000000, ATTR_MEM_NC);
#endif
#endif /* (USE_DDR == 1) */
#endif /* PS_PREINITED == 0 */
    
    
    init_uart();
    
    ret = FGicPs_CommonInit(&IntcInstance);
    if (ret != GIC_SUCCESS)
    {
        fmsh_print("GIC Setup Failed!\r\n");
    }
    else
    {
        fmsh_print("GIC Setup pass!\r\n");
    }
    
    if (ret != PSU_INIT_SUCCESS)
    {
        fmsh_print("PS_INIT_FAIL!\n\r");
        while (1)
        {
        };
    }

    Fmsh_ICacheEnable();

#if (DCACHE_ENABLE == 1)
    Fmsh_DCacheEnable();
#endif
}


//
u64 get_time_ms()
{
  
        #define COUNTS_PER_MILLI_SECOND (100000000/1000)


        global_timer_enable();
	
	u64 time;
	time=get_current_time();
	
	return (time/COUNTS_PER_MILLI_SECOND);


}


void cleanup_platform () {}
