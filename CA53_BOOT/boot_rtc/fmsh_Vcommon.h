#ifndef _FMSH_VCOMMON_H_
#define _FMSH_VCOMMON_H_

#include "fmsh_common.h"

typedef struct RegInstance 
{
    u16 offset;
    u8 RdWrOp;      //0--read; 1--read & write; 2--write only
    u32 active_bit;
    u32 reset_value;
}RegInstance_TypeDef;  
    
#endif    