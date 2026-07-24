/******************************************************************************
*
* Copyright (C) 2025 - 2035 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  main.c
*
* lwip
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   yyz  3/16/2025  First Release
*</pre>
******************************************************************************/
#include <stdio.h>
#include "fmsh_common.h"
#include "fmsh_gmacpsif.h"
#include "fmsh_gic.h"
#include "fmsh_lwipconfig.h"
#include "fadapter.h"
#include "fpga.h"
#include "platform.h"
#include "lwip/tcp.h"
#include "fmsh_sdmmc_example.h"
#include "ftpd.h"
#include "fmsh_gpio_public.h"
#include "shell.h"
#include "shell_port.h"
#include "release_rpu.h"
#include "load_img.h"
#include "fmsh_rtc_mix.h"
#include "eeprom_api.h"
#include "fmsh_hw.h"
#define APP_TYPE_UDP_PERF_CLIENT

extern short userShellWrite (char *data, unsigned short len);
extern short userShellRead (char *data, unsigned short len);
extern char shellBuffer[512];

/* missing declaration in lwIP */
void lwip_init();
static struct netif server_netif;
int fmsh_gmacif_input();

#ifdef APP_TYPE_UDP_PERF_CLIENT
int start_application_udp_perf_client();
void transfer_data_udp_perf_client(void);
void print_app_header_udp_perf_client(void);
#endif

FGpioPs_T gpio_bank2;
FATFS fs1; 

int main()
{
    ip_addr_t ipaddr, netmask, gw;
    /* the mac address of the board. this should be unique per board */
    unsigned char mac_ethernet_address[] =
    { 0x00, 0x0a, 0x35, 0x01, 0x02, 0x03 };
        

    u64 Pretime = 0;
    u64 Curtime = 0;
    u64 timeUsed = 0;
    unsigned char brk = 0;
    int index = 3000;
    int ret;
    u8 device_id = 0;
    const char *filename = "0:image";

    init_platform();
    global_timer_enable();
   // psu_ps_pl_isolation_removal_data();
   // psu_ps_pl_reset_config_data();
    ret = rtc_init();
    if (ret == 0) 
    fmsh_print("RTC init success\r\n");
    // psu_init();
    // delay_1ms();
    // FGpioPs_bank_init (FPAR_GPIOPS_2_DEVICE_ID, &gpio_bank2);
    // FGpioPs_setBitDirection(&gpio_bank2, Gpio_bit_11, Gpio_output);
    // FGpioPs_writeBit(&gpio_bank2, Gpio_low,Gpio_bit_11);
    // delay_ms(100);
    // FGpioPs_writeBit(&gpio_bank2, Gpio_high,Gpio_bit_11);
    // delay_ms(100);
    FSdPsu_fs_multi_partitions_example();

    lwip_init();

    //Configuring default IP of 192.168.255.10
    IP4_ADDR(&ipaddr,  10, 255,0,  2);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw,      10, 255,0,  10);
        
    /* Add network interface to the netif_list, and set it as default */
    fmsh_gmac_add(&server_netif, &ipaddr, &netmask,&gw, mac_ethernet_address,GMAC_SELECT_BASEADDR);
    netif_set_default(&server_netif);

    /* specify that the network if is up */
    netif_set_up(&server_netif);
        
    ftpd_init();

    initialize_rpu_tcm(FMZQ_CORE_RPU0);
    initialize_rpu_tcm(FMZQ_CORE_RPU1);
    
    eeprom_i2c_init();
#if 0
    u32 bit_size;
    ret = Load_PLBit(DDR_PL_ADDR, &bit_size);
    if(ret != FMSH_SUCCESS)
    {
        fmsh_print("Read fpga failed!\r\n");
        return FMSH_FAILURE;
    }
    
    ret = update_fpga(DDR_PL_ADDR, bit_size);
    if(ret != FMSH_SUCCESS)
    {
        fmsh_print("Load fpga failed!\r\n");
        return FMSH_FAILURE;
    }
    while(1);
#endif

#if 1

    while( index-- > 0 )
    {
        brk = uart_getc();
        if( brk ==  0x03 ) // Ctrl+C (ASCII 0x03)
        {
            fmsh_print("===================================================== \r\n");
            fmsh_print("Welcome to shell cmd environment. \r\n");
            fmsh_print("===================================================== \r\n");
            goto shell_cmd;
        }
    }

    u8 bandid=0xff;
    eeprom_get_bandinfo(&bandid);
    if(bandid > BAND_FREERTOS && bandid < BAND_LINUX_SAMPLE_MAX)
    {
        u32 MultiBootReg = 0;
        u32 RegValue = 0;
        FMSH_WriteReg(SAC_MULTI_BOOT_REG, 0x0, 0x30); 
        MultiBootReg = FMSH_ReadReg(SAC_MULTI_BOOT_REG, 0x0);
        
        fmsh_print("Multiboot register: 0x%x\r\n", MultiBootReg);
        RegValue = FMSH_ReadReg(CRL_APB_RESET_CTRL,0x0);
        FMSH_WriteReg(CRL_APB_RESET_CTRL, 0x0,
                RegValue | CRL_APB_RESET_CTRL_SOFT_RESET_MASK);
    }

    image_header_t header;
    if(emmc_read_image_header_version(device_id, &header)== FMSH_SUCCESS)
    {
        print_image_header_info(&header);
        for(LOAD_IMAGE_LIST img_list = 0; img_list < LOAD_IMAGE_UECONF; img_list++)
        {
            emmc_read_module_version(device_id, img_list);
        }
    }
    else
    {
        fmsh_print("Image header read fail!\r\n");
    }

    if(emmc_load_image(filename, device_id, LOAD_IMAGE_28DRBIT) == FMSH_SUCCESS)
    {
        if(emmc_load_image(filename, device_id, LOAD_IMAGE_CR50) == FMSH_SUCCESS)
        {
            fmsh_print("Delay 3s...\r\n");
            delay_ms(3000);
            if(emmc_load_image(filename, device_id, LOAD_IMAGE_CR51) == FMSH_SUCCESS)
            {
                if(check_and_create_ue_conf() == FMSH_SUCCESS)
                {
                    if(emmc_load_image(filename, device_id, LOAD_IMAGE_CA53) != FMSH_SUCCESS)
                    {
                        fmsh_print("Load ca53 failed!\r\n");
                        return FMSH_FAILURE;
                    }
                }
                else
                {
                    fmsh_print("ue.conf check failed!\r\n");
                }
            }
            else
            {
                fmsh_print("Load r51 failed!\r\n");
                return FMSH_FAILURE;
            }
        }
        else
        {
            fmsh_print("Load r50 failed!\r\n");
            return FMSH_FAILURE;
        }
    }
    else
    {
        fmsh_print("Load fpga failed!\r\n");
        return FMSH_FAILURE;
    }

shell_cmd:
#endif 
#if 1
    shell.write = userShellWrite;
    shell.read = userShellRead;
    // shell.lock = userShellLock;
    // shell.unlock = userShellUnlock;
    shellInit(&shell, shellBuffer, 512);
    while(1)
    {
        fmsh_gmacif_input(&server_netif);     
        shellTask(&shell);
    } 
#endif  
    return 0;
}
