#include <stdlib.h>
#include <Fatfs15/ff.h>
#include "fmsh_common.h"
#include "platform.h"
#include "psu_init.h"
#include "shell_port.h"
#include "fmsh_common_io.h"
#include "fpga.h"
#include "fmsh_qspi_example.h"
#include "fmsh_sdmmc_example.h"
#include "release_rpu.h"
#include "fmsh_rtc_common.h"
#include "fmsh_rtc_public.h"
#include "load_img.h"
#include "fmsh_rtc_mix.h"
#include "eeprom_main.h"
#include "boot_eeprom_api.h"

void writeRegs(u32 Offset, u32 Data)
{
    FMSH_WriteReg((u32)0, Offset, Data);
    fmsh_print("reg 0x%x value is 0x%x \r\n", Offset, FMSH_ReadReg((u32)Offset, 0));
}

void readRegs(u32 Offset)
{
    fmsh_print("reg 0x%x value is 0x%x \r\n",Offset, FMSH_ReadReg((u32)Offset, 0));
}

void load_ca53()
{
    u32 load_addr = DDR_A53_ADDR;
    Load_CA53(load_addr);
    Jump_to_bootloader(load_addr);
}

void load_fpga()
{   
    u32 bin_size = 0;
    u32 load_addr = DDR_PL_ADDR;
    Load_PLBit(load_addr, &bin_size);
    update_fpga(load_addr,bin_size);
}

void update_bootbin()
{
    int ret = FMSH_SUCCESS; 
    char *filename = "0:MOD_BOOT";
    u8 device_id = 0;
    ret = emmc_update_module(filename, device_id, LOAD_IMAGE_BOOT);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("emmc_update_module failed\r\n");
        remove_file(filename);
        return;
    }
    ret =  emmc_load_image(filename, device_id, LOAD_IMAGE_BOOT);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("emmc_load_image failed\r\n");
        remove_file(filename);
        return;
    }

    fmsh_print("emmc_load_image success\r\n");
}

void load_r5(u8 device_id)
{
    u8 nr = 0;
    u32 load_addr;
    
    nr = device_id + FMZQ_CORE_RPU0;
    switch (nr) 
    {
        case FMZQ_CORE_RPU0:
            load_addr = ADDR_R50;
            break;
        case FMZQ_CORE_RPU1:
            load_addr = ADDR_R51;
            break;
        default:
            fmsh_print("nr is %d !\r\n", nr);
            return;
    }

    fmsh_print("Enter %s, device_id is(%d) \r\n", __FUNCTION__, device_id);
    update_r5(device_id, load_addr);
    cpu_release(device_id+FMZQ_CORE_RPU0, load_addr);
    // while(1);
}

void cmd_cd(char *path)
{
    fmsh_print("cd  %s \r\n", path);
    f_chdir(path);
}


void cmd_ls(char *path)
{
    fmsh_print("Enter %s \r\n", __FUNCTION__);
    show_all_dir_of_partition(path);
    show_all_file_info_of_dir(path);

}

void mk_dir(char *path)
{
    fmsh_print("Enter %s \r\n", __FUNCTION__);
    f_mkdir(path);

}

void rm_file(char *path)
{
  fmsh_print("Enter %s \r\n", __FUNCTION__);
  remove_file(path);

}

void rm_dir(char *path)
{
  fmsh_print("Enter %s \r\n", __FUNCTION__);
  remove_all_dirs(path);

}

void cmd_cat(char *path)
{
  fmsh_print("Enter %s \r\n", __FUNCTION__);
  read_file(path);

}


void cmd_mkfs()
{
    u32 ulret;    
    DWORD plist[] = {0x1800000, 0, 0, 0}; 

    fdisk_physicaldrive(0);
    ulret = fmsh_SdEmmcInitPartFAT32(0, 1, plist);
    if (ulret)
    {
        fmsh_print("fmsh_SdEmmcInitPartFAT32 sdmmc0 err\r\n");
        return;
    }
    fmsh_print("fmsh_SdEmmcInitPartFAT32 sdmmc0 Success\r\n");
}

void cmd_showversion()
{
    u8 device_id = 0;
    image_header_t header;
    if(emmc_read_image_header_version(device_id, &header)== FMSH_SUCCESS)
    {
        print_image_header_info(&header);
        for(LOAD_IMAGE_LIST img_list = 0; img_list < LOAD_IMAGE_ALL-1; img_list++)
        {
            emmc_read_module_version(device_id, img_list);
        }
    }
    else
    {
        fmsh_print("Image header read  fail!\r\n");
    }
    return;
}

void cmd_update_img()
{
    const char *image_file = "0:image";
    u8 device_id = 0;
    if(emmc_update_image(image_file, device_id) == FMSH_SUCCESS)
    {
        fmsh_print("Update image Success!\r\n");
    }
    else
    {
        fmsh_print("Update image fail!\r\n");
    }
}


void cmd_load_image(u8 load_id)
{   
    const char *filename = "0:image";
    u8 device_id = 0;
    emmc_load_image(filename, device_id, load_id);

}

void cmd_update_module(char *filename, u8 load_id)
{   
    u8 device_id = 0;
    emmc_update_module(filename, device_id, load_id);

}

extern FRtcPs_T g_RTC;
void cmd_set_rtc_clock(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec)
{
    int ret = FMSH_SUCCESS; 
    rtc_time ctm;
    FRtcPs_portmap_T *portmap;  
    portmap = (FRtcPs_portmap_T *)g_RTC.base_address;

    if(year < 1980 || year > 2107)  
    {
        fmsh_print("Year param error! (1980~2107)\r\n");
        return;
    }
    if(mon < 1 || mon > 12)
    {
        fmsh_print("Month param error! (1~12)\r\n");
        return;
    }
    if(day < 1 || day > 31)
    {
        fmsh_print("Day param error! (1~31)\r\n");
        return;
    }
    if(hour > 23 || min >59 || sec >59)
    {
        fmsh_print("Time param error! (hour:0~23,min/sec:0~59)\r\n");
        return;
    }

    ctm.tm_year = year - 1900;
    ctm.tm_mon  = mon - 1;
    ctm.tm_mday = day;
    ctm.tm_hour = hour;
    ctm.tm_min  = min;
    ctm.tm_sec  = sec;

    ret = FRtcPs_set_time(&g_RTC, &ctm);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("FRtcPs_set_time failed\r\n");
        return;
    }

    FRtcPs_seconds_irq_enable(&g_RTC, 1);

    u32 s_val = 32768;
    u32 reg;
    FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, s_val);
    RTC_OUT32P(reg, portmap->CALIB_WRITE); 

    return;
}


void cmd_get_rtc_clock(void)
{
    int ret = FMSH_SUCCESS; 
    rtc_time ctm;

    ret =  FRtcPs_read_time(&g_RTC,  &ctm);

    fmsh_print("Time is %ld/%ld/%ld %2d:%2d:%2d \r\n", ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
    return;
}

void cmd_getbandinfo(void)
{
    u8 bandid=0xff;
    eeprom_get_bandinfo(&bandid);
    fmsh_print("bandid = 0x%x\r\n", bandid);
}

void cmd_setbandinfo(u8 bandid)
{
    if(bandid > BAND_LINUX_SAMPLE_MAX)
    {
        fmsh_print("Error! bandid = 0x%x, value out of range\r\n", bandid);
        return;
    }
    fmsh_print("bandid = 0x%x\r\n", bandid);
    eeprom_set_bandinfo(bandid);
}

void cmd_updatelinuxboot(void)
{
    u32 bin_size = 0;
    u32 load_addr = DDR_BOOTBIN_ADDR;
    Load_bootbin(load_addr, &bin_size);
    flash_update_bootbin2(load_addr, bin_size);

}

void cmd_read_eeprom_bin(u32 start_addr, u32 end_addr)
{
    char filename[128];
    int ret = FMSH_SUCCESS; 

    if(start_addr >= CONFIG_EEPROM_EEPROM_SIZE || end_addr >= CONFIG_EEPROM_EEPROM_SIZE || start_addr > end_addr)
    {
        fmsh_print("%s Addr out of range\r\n", __FUNCTION__);
        return;
    }
    memset(filename, 0, sizeof(filename));
    snprintf(filename, sizeof(filename), "0:/eeprom_0x%x_0x%x.bin", start_addr, end_addr);
    ret =  mw_eeprom_read_bin(start_addr, end_addr, filename);

    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("%s Read error\r\n", __FUNCTION__);
        return;
    }

}

void cmd_write_eeprom_bin(char *filename, u32 start_addr, u32 end_addr)
{
    int ret = FMSH_SUCCESS; 

    if(start_addr >= CONFIG_EEPROM_EEPROM_SIZE || end_addr >= CONFIG_EEPROM_EEPROM_SIZE || start_addr > end_addr)
    {
        fmsh_print("%s Addr out of range\r\n", __FUNCTION__);
        return;
    }

    ret =  mw_eeprom_write_bin(start_addr, end_addr, filename);

    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("%s Write error\r\n", __FUNCTION__);
        return;
    }

}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, writeRegs, writeRegs, para: offset and data);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, readRegs, readRegs, para: offset);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, load_fpga, load_fpga, load fpga);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, load_ca53, load_ca53, load A53);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, update_bootbin, update_bootbin, update boot.bin);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, load_r5, load_r5, para: device_id(0(r50) 1(r51)));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, cd, cmd_cd, para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, ls, cmd_ls, para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, mkdir, mk_dir, para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, rmfile, rm_file, para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, rmdir,  rm_dir,  para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, cat,  cmd_cat,  para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, mkfs,  cmd_mkfs, Format the filesystem);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, set_rtc_clock, cmd_set_rtc_clock, para:year mon day hour min sec (2026 01 19 14 30 20)--- set rtc clock);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, get_rtc_clock, cmd_get_rtc_clock, get rtc clock);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, showversion, cmd_showversion, show version);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, update_img, cmd_update_img, update img);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, load_img, cmd_load_image, para:id(0(boot) 1(a53) 2(r50) 3(r51) 4(bit)));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, update_module, cmd_update_module, para: filename id(0(boot) 1(a53) 2(r50) 3(r51) 4(bit)));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, check_and_create_ue_conf, check_and_create_ue_conf, (void));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, getbandinfo, cmd_getbandinfo, (void));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, setbandinfo, cmd_setbandinfo, bandid(0x0, 0x3, 0x4));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, updatelinuxboot, cmd_updatelinuxboot, (void));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, read_eeprom_bin, cmd_read_eeprom_bin, para: start_addr end_addr);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN, write_eeprom_bin, cmd_write_eeprom_bin, para: filename start_addr end_addr);

void run_shell(void) {
  userShellInit();
}
