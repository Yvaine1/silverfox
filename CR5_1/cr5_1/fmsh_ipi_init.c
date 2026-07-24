
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
#include "semphr.h"

IpiPsu Ipi_a53;
IpiPsu Ipi_cr5_0;

__attribute__((unused)) static u32 RecvRemoteId;
__attribute__((unused)) static u32 RecvFlagA53;
__attribute__((unused)) static u32 RecvFlagCR5_0;

extern SemaphoreHandle_t xShellIpiSem;

void Ipi_Irq_Handler_A53(void *InstancePtr)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    u32 IntrStatus;
    IpiPsu *Ipi_Instance_Ptr = (IpiPsu *)InstancePtr;

    IntrStatus = IpiPsu_GetInterruptStatus(Ipi_Instance_Ptr);
    IpiPsu_ClearInterruptStatus(Ipi_Instance_Ptr, IntrStatus);

    if (xShellIpiSem != NULL)
    {
        xSemaphoreGiveFromISR(xShellIpiSem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void Ipi_Irq_Handler_CR5_0(void *InstancePtr)
{
    IpiPsu *Ipi_Instance_Ptr = (IpiPsu *)InstancePtr;
    u32 IntrStatus;
    
    IntrStatus = IpiPsu_GetInterruptStatus(Ipi_Instance_Ptr);
    IpiPsu_ClearInterruptStatus(Ipi_Instance_Ptr, IntrStatus);
    RecvFlagCR5_0 = 1;
}

static void IrqTargetConfig (FGicPs *InstancePtr,u32 DeviceId)
{
    IpiPsu_Config *CfgPtr;

    CfgPtr = IpiPsu_LookupConfig(DeviceId);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 0x2, CfgPtr->IntId);
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

static const struct {
    uint64_t size;
    uint32_t encoding;
}region_size[] = {
    { 0x20, REGION_32B },
    { 0x40, REGION_64B },
    { 0x80, REGION_128B },
    { 0x100, REGION_256B },
    { 0x200, REGION_512B },
    { 0x400, REGION_1K },
    { 0x800, REGION_2K },
    { 0x1000, REGION_4K },
    { 0x2000, REGION_8K },
    { 0x4000, REGION_16K },
    { 0x8000, REGION_32K },
    { 0x10000, REGION_64K },
    { 0x20000, REGION_128K },
    { 0x40000, REGION_256K },
    { 0x80000, REGION_512K },
    { 0x100000, REGION_1M },
    { 0x200000, REGION_2M },
    { 0x400000, REGION_4M },
    { 0x800000, REGION_8M },
    { 0x1000000, REGION_16M },
    { 0x2000000, REGION_32M },
    { 0x4000000, REGION_64M },
    { 0x8000000, REGION_128M },
    { 0x10000000, REGION_256M },
    { 0x20000000, REGION_512M },
    { 0x40000000, REGION_1G },
    { 0x80000000, REGION_2G },
    { 0x100000000, REGION_4G },
};

uint32_t SetMPURegion(uint32_t addr, uint64_t size, int region_num,uint32_t attrib)
{
	uint32_t local_addr = addr;
	uint32_t region_val;
	int i;

	local_addr &= (uint32_t)(~(size - 1U)); /* align */
	size += addr - local_addr;
	
    /* Lookup the size.  */
    for (i = 0; i < (sizeof (region_size) / sizeof (region_size[0])); i++) {
        if (size <= region_size[i].size) {
            region_val = region_size[i].encoding;
            break;
        }
    }

    Fmsh_SetAttribute(addr, region_val, region_num, attrib);
	
	return 0;
}

void ipi_a53_2_r51_init()
{
    IPI_Init(&Ipi_a53,IPI_ID_CH2,(FMSH_InterruptHandler)Ipi_Irq_Handler_A53);
}

void ipi_demo()
{
    IPI_Init(&Ipi_a53,IPI_ID_CH2,(FMSH_InterruptHandler)Ipi_Irq_Handler_A53);
    IPI_Init(&Ipi_cr5_0,IPI_ID_CH10,(FMSH_InterruptHandler)Ipi_Irq_Handler_CR5_0);
    SetMPURegion(SHMEM_ADDR_A53_TO_CR5_0,TOTAL_SHMEM_SIZE,8,NORM_SHARED_NCACHE|PRIV_RW_USER_RW);
    
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




void prvRecvMessage_1 (void *pvParameters)
{
    u32 recv_a53[20];
    u32 recv_cr5_0[20];
    
    u32 send_a53[20];
    u32 send_cr5_0[20];
    
    ipi_demo();
    
    for (;;)
    {
        if (RecvFlagA53)
        {
            RecvFlagA53 = 0;
            Recv_Message(recv_a53,20,SHMEM_ADDR_A53_TO_CR5_0);
            fmsh_print("rpu1:recv a53:\n\r");
            dump_message(recv_a53,20);
            
            shmem_msg_construct(send_a53,20,SHMEM_ADDR_CR5_0_TO_A53);
            Send_Message(&Ipi_a53,send_a53,20,SHMEM_ADDR_CR5_0_TO_A53,REMOTE_MASK_CH8);
        }
        
        if (RecvFlagCR5_0)
        {
            RecvFlagCR5_0 = 0;
            Recv_Message(recv_cr5_0,20,SHMEM_ADDR_CR5_1_TO_0);
            fmsh_print("rpu1:recv rpu0:\n\r");
            dump_message(recv_cr5_0,20);
            shmem_msg_construct(send_cr5_0,20,SHMEM_ADDR_CR5_0_TO_1);
            Send_Message(&Ipi_cr5_0,send_cr5_0,20,SHMEM_ADDR_CR5_0_TO_1,REMOTE_MASK_CH10);
        }
    }
}













