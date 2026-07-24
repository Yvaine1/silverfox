#include "fmsh_cci_reg.h"

#define C28DR_IDCODE 0x147E0093
#define C2EG_IDCODE 0x14711093
IpiPsu IpiInst; /* IPI driver instance */

static u32 Channel_bit_transfer(int Ch_Num)
{
    u32 num,val;

    num = Ch_Num;
    switch(num) {
     case 0:  val = 0x1;break;
     case 1:  val = 0x100;break;
     case 2:  val = 0x200;break;
     case 3:  val = 0x10000;break; 
     case 4:  val = 0x20000;break;
     case 5:  val = 0x40000;break;
     case 6:  val = 0x80000;break;
     case 7:  val = 0x1000000;break;
     case 8:  val = 0x2000000;break; 
     case 9:  val = 0x4000000;break;
     case 10: val = 0x8000000;break;
     default: val = 0x1;
    }

    return val;
}

u32 IpiInit(IpiPsu *IpiInstPtr)
{
    u32 Status = FMSH_FAILURE;
    IpiPsu_Config *CfgPtr;

    CfgPtr = IpiPsu_LookupConfig(0U);
    if (CfgPtr == NULL) {
        goto END;
    }

    Status = IpiPsu_CfgInitialize(IpiInstPtr, CfgPtr, CfgPtr->BaseAddress);
    if (Status != FMSH_SUCCESS) {
        goto END;
    }       

END:
    return Status;
} 

static u32 CCIreg_IpiSender(IpiPsu *IpiInstPtr,int srcCH,int destCH)
{
    u32 Status = FMSH_FAILURE;
    u32 destBit;
    destBit = Channel_bit_transfer(destCH);
    u32 Buffer[8] = {0xF0001,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
    //step1 : Write message 
    Status = IpiPsu_WriteMessage(IpiInstPtr,destBit,Buffer,8,IPIPSU_BUF_TYPE_MSG);
    if (Status != FMSH_SUCCESS) {
        return Status;
    }

    //step2:Trigeer IPI interrupt
    Status =  IpiPsu_TriggerIpi(IpiInstPtr,destBit);
    if (Status != FMSH_SUCCESS) {
        return Status;
    }
    FMSH_WriteReg(0xFF410000, 0x4C, 0x2U);
    asm("wfi");  
    return Status;
}   
static u32 CCIreg_Is_V01(void)
{
    u32 IDCODE = FMSH_ReadReg(0xFFCA0000, 0x720);/* Read IDCODE */
    if((IDCODE == C28DR_IDCODE) || (IDCODE == C2EG_IDCODE))
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}
u32 CCIreg_init(void)
{
    u32 Status = FMSH_FAILURE;
    if(CCIreg_Is_V01() && (FMSH_ReadReg(0xFF410000, 0x4CU) == 0x0U) && (FMSH_ReadReg(0xFF5E0000, 0x104)==0x01)){
        Status = IpiInit(&IpiInst);
        if(Status != FMSH_SUCCESS) {
            return Status;
        }
        delay_ms(3);
        CCIreg_IpiSender(&IpiInst,0,3);
    }
    return Status;
}
                
                
                
                


