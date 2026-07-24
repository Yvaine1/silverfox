
#ifndef __FMSH_IPI_TEST_H_
#define __FMSH_IPI_TEST_H_

typedef struct __attribute__((packed))
{
    unsigned int r[13];
    unsigned int lr;
    unsigned int sp;
    unsigned int spsr;
    unsigned int cpsr;
    unsigned int exc_lr;
    unsigned int pc;
} RegContext_t;

void backtrace_exc(void);

#endif
