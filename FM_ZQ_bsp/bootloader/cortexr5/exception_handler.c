/******************************************************************************
 *
 * (c) Copyright 2009-13 FMSH, Inc. All rights reserved.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file exception_handler.c
 *
 * This file contains the exception handler for the processor
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------- -------- ---------------------------------------------------
 * 1.00a liulei  19/12/06 Initial version
 * 1.01  hzq     22/08/16 use fmsh_print instead of printf.
 * 1.02  hzq     22/11/14 enable irq and fiq in FMSH_ExceptionRegisterHandler
 * 2.00  hzq     22/11/22 add cortexr5 support
 * </pre>
 *
 * @note
 *
 * None.
 *
 ******************************************************************************/
#include <stdio.h>

#include "cortexr5/cortexr5.h"
#include "exception_handler.h"
#include "fmsh_pseudo_asm.h"
#include "backtrace.h"

typedef struct {
    FMSH_ExceptionHandler Handler;
    void *Data;
} FExc_VectorTableEntry;

static void FMSH_ExceptionNullHandler(void *Data);

static const char *abort_status[][2] = {
    // IFSR status,,                                      DFSR status
    {"Unknown(reserved status)", "Unknown(reserved status)"},        // 0
    {"Alignment Fault", "Alignment Fault"},                          // 1
    {"Debug Event", "Debug Event"},                                  // 2
    {"Background", "Background"},                                    // 3
    {"Permission", "Permission"},                                    // 4
    {"Synchronous external abort", "Synchronous external abort"},    // 5
    {"Asynchronous external abort", "Asynchronous external abort"},  // 6
    {"Synchronous Parity or ECC Error",
     "Synchronous Parity or ECC Error"},                             // 7
    {"Asynchronous Parity or ECC Error",
     "Asynchronous Parity or ECC Error"},                            // 8
};

u32 UndefinedExceptionAddr;
u32 DataAbortAddr;
u32 PrefetchAbortAddr;

/****************************************************************************/
/**
 * The function is a common API used to initialize exception handlers across all
 * processors supported. The exception handlers are being
 * initialized statically and hence this function does not do anything.
 * However, it is still present to avoid any compilation issues in case an
 * application uses this API and also to take care of backward compatibility
 * issues (in earlier versions of BSPs, this API was being used to initialize
 * exception handlers).
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *****************************************************************************/
void FMSH_ExceptionInit (void) { return; }

/*****************************************************************************/
/**
*
* Makes the connection between the Id of the exception source and the
* associated Handler that is to run when the exception is recognized. The
* argument provided in this call as the Data is used as the argument
* for the Handler when it is called.
*
* @param	exception_id contains the ID of the exception source and should
*		be in the range of 0 to FMSH_EXCEPTION_ID_LAST.
See FMSH_exception_l.h for further information.
* @param	Handler to the Handler for that exception.
* @param	Data is a reference to Data that will be passed to the
*		Handler when it gets called.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void FMSH_ExceptionNullHandler (void *Data)
{
  while(1){};
}

FExc_VectorTableEntry FExc_VectorTable[FMSH_EXCEPTION_ID_LAST + 1] = {
    {FMSH_ExceptionNullHandler, NULL}, {FMSH_ExceptionNullHandler, NULL},
    {FMSH_ExceptionNullHandler, NULL}, {FMSH_ExceptionNullHandler, NULL},
    {FMSH_ExceptionNullHandler, NULL}, {FMSH_ExceptionNullHandler, NULL},
    {FMSH_ExceptionNullHandler, NULL},
};

void FMSH_ExceptionRegisterHandler (u32 Exception_id,
                                    FMSH_ExceptionHandler Handler, void *Data)
{
    FExc_VectorTable[Exception_id].Handler = Handler;
    FExc_VectorTable[Exception_id].Data = Data;

    if (Exception_id == FMSH_EXCEPTION_ID_IRQ_INT)
    {
        FMSH_ExceptionEnable();
    }
    else if (Exception_id == FMSH_EXCEPTION_ID_FIQ_INT)
    {
        FMSH_FastInterruptEnable();
    }
    else
    {
    }
}

/************************************************************************/
void UndefinedException (RegContext_t *ctx)
{
    ctx->pc = ctx->exc_lr - 4;

    fmsh_print(
        "\n\r##################################################################"
        "####\n\r");
    fmsh_print("Undefined Exception, occured at [0x%08x]\r\n",ctx->pc);
    backtrace_exc();
    while (1){};
}

void DataAbortInterrupt (RegContext_t *ctx)
{
    uint32_t v1, v2, dfsr, idx;
    ctx->pc = ctx->exc_lr - 8;
    mfcp(CP15_DATA_FAULT_STATUS, v1);
    mfcp(CP15_DATA_FAULT_ADDRESS, v2);

    dfsr = ((v1 >> 4) & 0x0F);
    fmsh_print(
        "\n\r##################################################################"
        "####\n\r");
    fmsh_print("Data Abort Exception, occured in %x domain, ", dfsr);
    dfsr = (((v1 & 0x400) >> 6) | (v1 & 0x0F));
    switch (dfsr)
    {
    case 0x00:
        idx = 3;
        break;
    case 0x01:
        idx = 1;
        break;
    case 0x02:
        idx = 2;
        break;
    case 0x08:
        idx = 5;
        break;
    case 0x0d:
        idx = 4;
        break;
    case 0x16:
        idx = 6;
        break;
    case 0x18:
        idx = 7;
        break;
    case 0x19:
        idx = 8;
        break;
    default:
        idx = 0;
        break;
    }
    fmsh_print("reason is: %s\n\r", (char *)abort_status[idx][1]);
    fmsh_print("Data fault occured at Address = [0x%08x]\n\n\r", v2);
    fmsh_print("-[Info]-Data fault status register value = 0x%x\n\r", v1);

    backtrace_exc();
    while (1);
}

void PrefetchAbortInterrupt (RegContext_t *ctx)
{
    uint32_t v1, v2, ifsr, idx;
    ctx->pc = ctx->exc_lr - 4;

    mfcp(CP15_INST_FAULT_STATUS, v1);
    mfcp(CP15_INST_FAULT_ADDRESS, v2);

    ifsr = (((v1 & 0x400) >> 6) | (v1 & 0x0F));
    switch (ifsr)
    {
    case 0x00:
        idx = 3;
        break;
    case 0x01:
        idx = 1;
        break;
    case 0x02:
        idx = 2;
        break;
    case 0x08:
        idx = 5;
        break;
    case 0x0d:
        idx = 4;
        break;
    case 0x16:
        idx = 6;
        break;
    case 0x18:
        idx = 7;
        break;
    case 0x19:
        idx = 8;
        break;
    default:
        idx = 0;
        break;
    }
    fmsh_print(
        "\n\r##################################################################"
        "####\n\r");
    fmsh_print("Instruction prefetch abort Exception, reason is: %s\n\r",
               (char *)abort_status[idx][0]);
    fmsh_print("Instruction prefetch Fault occured at Address = [0x%08x]\n\r", v2);
    fmsh_print("-[INFO]- Prefetch Fault status register value by = [0x%08x]\n\r", v1);

    backtrace_exc();
    while (1){};
}

void SWInterrupt (u32 value)
{
    fmsh_print_info("SWI occured at [0x%08x]\r\n", value);

    while (1){};
}

/*****************************************************************************/
/**
 * This is the C level wrapper for the FIQ interrupt called from the vectors.s
 * file.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void IRQInterrupt (void)
{
    FExc_VectorTable[FMSH_EXCEPTION_ID_IRQ_INT].Handler(
        FExc_VectorTable[FMSH_EXCEPTION_ID_IRQ_INT].Data);
}

/*****************************************************************************/
/**
 *
 * This is the C level wrapper for the IRQ interrupt called from the vectors.s
 * file.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FIQInterrupt (void)
{
    FExc_VectorTable[FMSH_EXCEPTION_ID_FIQ_INT].Handler(
        FExc_VectorTable[FMSH_EXCEPTION_ID_FIQ_INT].Data);
}
