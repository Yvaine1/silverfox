
#ifndef __FTOPOLOGY_H_
#define __FTOPOLOGY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fmsh_gmacpsif.h"
struct ftopology_t {
	unsigned gmac_baseaddr;
	enum fgmac_types gmac_type;
	unsigned intc_baseaddr;
	unsigned intc_gmac_intr;	
	unsigned scugic_baseaddr; 
	unsigned scugic_gmac_intr; 
};
extern struct ftopology_t ftopology[];





#ifdef __cplusplus
}
#endif

#endif
