#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__


 #include "stdio.h"
#include "lwipopts.h"

 #define LWIP_NO_STDINT_H  1

 typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   int    u32_t;
typedef signed     int    s32_t;
typedef unsigned   long long    u64_t;
typedef signed     long long    s64_t;

#define S16_F "d"
#define U16_F "d"
#define S32_F "d"
#define U32_F "x"

#define X16_F "x"
#define X32_F "x"

#define LWIP_RAND rand

typedef unsigned long mem_ptr_t;

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_PLATFORM_ASSERT(x)
#define LWIP_PLATFORM_DIAG(x) do { fmsh_print x; } while(0)

#ifndef BYTE_ORDER
#ifdef PROCESSOR_LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif




#endif /* __ARCH_CC_H__ */