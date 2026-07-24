
#ifndef PORTABLE_H
#define PORTABLE_H

#include <ysizet.h>

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */
        
void * pvPortMalloc(size_t size);
void pvPortFree(void* point);

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* PORTABLE_H */