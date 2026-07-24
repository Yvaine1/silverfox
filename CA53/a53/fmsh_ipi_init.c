
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "fmsh_gic.h"
#include "mem_common.h"
#include "platform.h"
#include "fmsh_ipi.h"
#include "fmsh_common.h"
#include "fmsh_gic_hw.h"
#include "fmsh_ipi_init.h"
#include "fmsh_common_types.h"
#include "fmsh_mailbox_ipips.h"

IpiPsu Ipi_a53_cr5_0;
IpiPsu Ipi_a53_cr5_1;

__attribute__((unused)) static u32 RecvRemoteId;
__attribute__((unused)) static u32 RecvFlagCR5_0;
__attribute__((unused)) static u32 RecvFlagCR5_1;

void Ipi_Irq_Handler_CR5_0(void *InstancePtr)
{
    IpiPsu *Ipi_Instance_Ptr = (IpiPsu *)InstancePtr;
    u32 IntrStatus;
    
    IntrStatus = IpiPsu_GetInterruptStatus(Ipi_Instance_Ptr);
    IpiPsu_ClearInterruptStatus(Ipi_Instance_Ptr, IntrStatus);
    
    RecvFlagCR5_0 = 1;
}

void Ipi_Irq_Handler_CR5_1(void *InstancePtr)
{
    IpiPsu *Ipi_Instance_Ptr = (IpiPsu *)InstancePtr;
    u32 IntrStatus;
    
    IntrStatus = IpiPsu_GetInterruptStatus(Ipi_Instance_Ptr);
    IpiPsu_ClearInterruptStatus(Ipi_Instance_Ptr, IntrStatus);
    RecvFlagCR5_1 = 1;
}

static void IrqTargetConfig (FGicPs *InstancePtr,u32 DeviceId)
{
    IpiPsu_Config *CfgPtr;

    CfgPtr = IpiPsu_LookupConfig(DeviceId);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 0x1, CfgPtr->IntId);
    }
}

u32 IPI_Init(IpiPsu *Ipi_Instance_Ptr,u32 DeviceId, FMSH_InterruptHandler Message_Handler)
{
    u32 Status = FMSH_FAILURE;
    IpiPsu_Config *CfgPtr;

    IrqTargetConfig(&IntcInstance,DeviceId);
    
    CfgPtr = IpiPsu_LookupConfig(DeviceId);
    if (NULL == CfgPtr)
    {
        return Status;
    }
    
    Status = IpiPsu_CfgInitialize(Ipi_Instance_Ptr, CfgPtr, CfgPtr->BaseAddress);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    /* Enable reception of IPI from all CPUs */
    IpiPsu_InterruptEnable(Ipi_Instance_Ptr, IPIPSU_ALL_MASK);
    
    /* Clear Any existing Interrupts */
    IpiPsu_ClearInterruptStatus(Ipi_Instance_Ptr, IPIPSU_ALL_MASK);
    
    Status = FGicPs_Connect(&IntcInstance, CfgPtr->IntId,Message_Handler,
                            (void *)Ipi_Instance_Ptr);
    
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    
    FGicPs_Enable(&IntcInstance, CfgPtr->IntId);

    return FMSH_SUCCESS;
}

void Send_Message(IpiPsu *InstancePtr,u32 *msg, u32 len, u32 data_addr, u32 Remote_Id)
{
    u32 Index;
    for (Index = 0; Index < len; Index++)
    {
        *((u32 *)(data_addr)) = msg[Index];
        data_addr += 4;
    }

    IpiPsu_TriggerIpi(InstancePtr, Remote_Id);
}

void Recv_Message(u32 *msg, u32 len,u32 data_addr)
{
    u32 Index;
  
    /* Copy the IPI Buffer contents into Users's Buffer*/
    for (Index = 0U; Index < len; Index++)
    {
        msg[Index] = *((u32 *)(data_addr));
        data_addr += 4;
    }
}

void shmem_msg_construct(u32 *msg,u32 len,u32 data_addr)
{
  int i;
  TickType_t tick_count = xTaskGetTickCount();
   
  srand((unsigned int)(tick_count));
  
  for (i = 0; i < len; i++)
  {
     msg[i] = data_addr + i + rand() % 100;
  }
  
}

void dump_message(u32 *msg, u32 len)
{
   u32 i;
   
   for(i = 0; i < len; i++)
   {
     fmsh_print(" -0x%0x", msg[i]);
   }
   
   fmsh_print("\n\r");
}

void ipi_a53_2_r5_init()
{
    IPI_Init(&Ipi_a53_cr5_0,IPI_ID_CH7,(FMSH_InterruptHandler)Ipi_Irq_Handler_CR5_0);
    IPI_Init(&Ipi_a53_cr5_1,IPI_ID_CH8,(FMSH_InterruptHandler)Ipi_Irq_Handler_CR5_1);
}

void ipi_demo()
{
    IPI_Init(&Ipi_a53_cr5_0,IPI_ID_CH7,(FMSH_InterruptHandler)Ipi_Irq_Handler_CR5_0);
    IPI_Init(&Ipi_a53_cr5_1,IPI_ID_CH8,(FMSH_InterruptHandler)Ipi_Irq_Handler_CR5_1);
    Fmsh_SetTlbAttributesRange(SHMEM_ADDR_A53_TO_CR5_0,TOTAL_SHMEM_SIZE - 1,ATTR_MEM_NC);
    
//    while(RecvFlagCR5_0 != 1);
// 
//    Recv_Message(recv_cr5_0,20,SHMEM_ADDR_CR5_0_TO_A53);
//    fmsh_print("a53:recv rpu0:\n\r");
//    dump_message(recv_cr5_0,20);
//    shmem_msg_construct(send_cr5_0,20,SHMEM_ADDR_A53_TO_CR5_0);
//    Send_Message(&Ipi_a53_cr5_0,send_cr5_0,20,SHMEM_ADDR_A53_TO_CR5_0,REMOTE_MASK_CH1);
//    
//    
//    while(RecvFlagCR5_1 != 1);
//       
//    Recv_Message(recv_cr5_1,20,SHMEM_ADDR_CR5_1_TO_A53);
//    fmsh_print("a53:recv rpu1:\n\r");
//    dump_message(recv_cr5_1,20);
//    shmem_msg_construct(send_cr5_1,20,SHMEM_ADDR_A53_TO_CR5_1);
//    Send_Message(&Ipi_a53_cr5_1,send_cr5_1,20,SHMEM_ADDR_A53_TO_CR5_1,REMOTE_MASK_CH2);
//    
//    while(1);
}


void prvSendRpuMessage (void *pvParameters)
{
//    u64 mpidr;
    u32 send_cr5_0[20];
    u32 send_cr5_1[20];
    
    for (;;)
    {
        vTaskDelay(3000);
        shmem_msg_construct(send_cr5_0,20,SHMEM_ADDR_A53_TO_CR5_0);
        Send_Message(&Ipi_a53_cr5_0,send_cr5_0,20,SHMEM_ADDR_A53_TO_CR5_0,REMOTE_MASK_CH1);
        shmem_msg_construct(send_cr5_1,20,SHMEM_ADDR_A53_TO_CR5_1);
        Send_Message(&Ipi_a53_cr5_1,send_cr5_1,20,SHMEM_ADDR_A53_TO_CR5_1,REMOTE_MASK_CH2);
//        mfcp(MPIDR_EL1, mpidr);
//        mpidr = mpidr & 0xff;
//        fmsh_print("Send Message (core %llx) \r\n",mpidr);
        
    }
}

void prvRecvRpuMessage (void *pvParameters)
{
//    u64 mpidr;
    u32 recv_cr5_0[20];
    u32 recv_cr5_1[20]; 
    
    for (;;)
    {
        if (RecvFlagCR5_0)
        {
            RecvFlagCR5_0 = 0;
            fmsh_print("a53:recv rpu0:\n\r");
            dump_message(recv_cr5_0,20);
        }
        
        if (RecvFlagCR5_1)
        {
            RecvFlagCR5_1 = 0;
            fmsh_print("a53:recv rpu1:\n\r");
            dump_message(recv_cr5_1,20);
        }
//        mfcp(MPIDR_EL1, mpidr);
//        mpidr = mpidr & 0xff;
//        fmsh_print("Recv Message (core %llx) \r\n",mpidr);
    }
}













