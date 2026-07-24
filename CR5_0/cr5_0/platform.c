/*
 * Uncomment one of the following two lines, depending on the target,
 * if ps7/psu init source files are added in the source directory for
 * compiling example outside of IAR.
 */
#include "fmsh_common.h"
#include "fmsh_print.h"
#include "fmsh_psu_parameters.h"
#include "psu_init.h"

void init_platform ()
{
    int ret = PSU_INIT_SUCCESS;

#if PS_PREINITED == 0
    ret = psu_init();
#if USE_DDR == 1
    Fmsh_SetAttribute(0x0, REGION_2G, 0, NORM_NSHARED_WB_WA | PRIV_RW_USER_RW);
#endif /* (USE_DDR == 1) */
#endif /* PS_PREINITED == 0 */

    init_uart();

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

void cleanup_platform () {}
