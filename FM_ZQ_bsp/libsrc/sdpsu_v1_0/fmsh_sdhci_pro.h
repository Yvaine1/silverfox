#ifndef _FMSH_SDHCI_PRO_H_
#define _FMSH_SDHCI_PRO_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "fmsh_common.h"
#if !NO_OS

int emmc_mutex_init(void);
int emmc_mutex_delete(void);
int emmc_lock(uint32_t timeout_ms);
void emmc_unlock(void);

#endif /* !NO_OS */
#ifdef __cplusplus
}
#endif

#endif