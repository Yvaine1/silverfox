/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */

#include "dg_common.h"
#include "fmsh_common.h"
#include "fmsh_common_io.h"
#include "fmsh_ps_parameters.h"

#define EMIO_BASE_ADDR  0xe0003000
#define REG_DATA_ADDR (0x200)
#define REG_DIRM_ADDR (0x204)
#define REG_R_DATA_ADDR (0x250)
#define PL_RESET_EMIO (22) //emio 22

#define HP_REG_BASE	(0xe0029000)
#define HP_REG_SIZE	(0x1000)

static void set_fpga_pl_reset_prog_l(int val)
{
    // CONF_CPU_FPGA_PROG_L ?EMIO22
    int reg_val = 0;
    reg_val = FMSH_ReadReg(EMIO_BASE_ADDR, REG_DATA_ADDR);

    if (val == 1)
    {
        reg_val |= ((int)1 )<<PL_RESET_EMIO;// set to 1
    }
    else
    {
        reg_val &= ~(((int)1 )<<PL_RESET_EMIO);//set to 0
    }

    FMSH_WriteReg(EMIO_BASE_ADDR, REG_DATA_ADDR, reg_val);
}

void pl_reset(void)
{
    unsigned int regDir = 0;
    regDir = FMSH_ReadReg(EMIO_BASE_ADDR,REG_DIRM_ADDR);
    regDir |= (1 << PL_RESET_EMIO);
    FMSH_WriteReg(EMIO_BASE_ADDR,REG_DIRM_ADDR,regDir);

    set_fpga_pl_reset_prog_l(0);
    delay_us(20);
    set_fpga_pl_reset_prog_l(1);
    
    hp_re_enable();
}   

void hp_re_enable(void)
{
    FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x008, 0xDF0D767B);
    FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x838, 0xf);
    
    delay_ms(50);
    FMSH_WriteReg(HP_REG_BASE, 0x490, 0x1);
    FMSH_WriteReg(HP_REG_BASE, 0x490+0xb0, 0x1);
    FMSH_WriteReg(HP_REG_BASE, 0x490+0xb0*2, 0x1);
    FMSH_WriteReg(HP_REG_BASE, 0x490+0xb0*3, 0x1);
    
    FMSH_WriteReg(FPS_SLCR_BASEADDR,0x004, 0xDF0D767B);
}

void enable_user_level_shift(void)
{
    FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x008, 0xDF0D767B);
    FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x838, 0xf);
    FMSH_WriteReg(FPS_SLCR_BASEADDR,0x004, 0xDF0D767B);
}

void disable_user_level_shift(void)
{
    FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x008, 0xDF0D767B);
    FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x838, 0x0);
    FMSH_WriteReg(FPS_SLCR_BASEADDR,0x004, 0xDF0D767B);
}



