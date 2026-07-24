#include <metal/assert.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"
#include "platform.h"
#include "xparameters.h"
#include "xrfdc.h"
#include "fmsh_common.h"
#include "cx4e04_config.h"
#include "shell_port.h"
#include "xrfdc_main.h"
#include "fmsh_common_types.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"


#ifdef __BAREMETAL__
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#define I2CBUS	1
#else
#define RFDC_DEVICE_ID 	0
#define I2CBUS	12
#endif
void FI2cPs_example2(void);
extern void run_shell(void);
extern void send_singleecho(void);
extern void metal_rfdc_writeRegs(UINT32 Offset, UINT32 Data);
extern void metal_rfdc_readRegs(UINT32 Offset);

#define SGI_ID 0U
#define CPU_ID 1U


void SGI0_hanlder(void *InstancePtr);

u32 gicTestFlag = GIC_FAILURE;



u32 FGicPs_CommonInit (FGicPs *InstancePtr)
{
    u32 Status;

    Status = FGicPs_SetupInterruptSystem(InstancePtr);
    if (Status != GIC_SUCCESS)
    {
        return GIC_FAILURE;
    }

    FMSH_ExceptionRegisterHandler(
        FMSH_EXCEPTION_ID_IRQ_INT,
        (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ, InstancePtr);

    return Status;
}


void SGI0_hanlder (void *InstancePtr) { gicTestFlag = GIC_SUCCESS; }

void metal_rfdc_init()
{
        rfdc_init(RFDC_DEVICE_ID);
        rfdc_reset(0,0);
        rfdc_reset(0,2);
        rfdc_reset(1,0);
        rfdc_reset(1,1);
        send_singleecho();
}

int rfdc_main(void)
{
        init_platform();
	int ret = 0;
        cx4e04_init();
        fmsh_print("\r\n");
        
        psu_ps_pl_isolation_removal_data();
        psu_ps_pl_reset_config_data();

        ret = FGicPs_CommonInit(&IntcInstance);
        if (ret != GIC_SUCCESS)
        {
            fmsh_print("GIC Setup Failed!\r\n");
        }
        else
        {
            fmsh_print("GIC Setup pass!\r\n");
        }


//        metal_rfdc_writeRegs(0x28c,0x1);//xdma�ػ�����
//        axidma_main();

        
/*
        i2c_dac80501_set_v_out(2);
        i2c_dac80501_get_dac_reg();
        i2c_dac80501_get_device_reg();
*/
        //run_shell();
       
        fmsh_print("Successfully ran MTS Example\r\n");

	return ret;
}
