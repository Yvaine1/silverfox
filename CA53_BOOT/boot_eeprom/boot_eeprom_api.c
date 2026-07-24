/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fmsh_common.h"
#include "boot_eeprom_api.h"
#include "eeprom_api.h"
#include "ff.h"

int mw_eeprom_read_bin(u32 start_addr, u32 end_addr, const char *cnf_file)
{
    int ret = FMSH_SUCCESS;
    FIL fp;

    ret = f_open(&fp, cnf_file, FA_CREATE_ALWAYS | FA_WRITE);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Can't creat binary file.\r\n");
        return FMSH_FAILURE;
    }
    u32 current = start_addr;

    while (current <= end_addr) 
    {
        u32 bytes_written = 0;
        u32 page_end = (current & ~(CONFIG_EEPROM_PAGE_SIZE - 1)) + CONFIG_EEPROM_PAGE_SIZE - 1;
        u32 read_end = (end_addr < page_end) ? end_addr : page_end;
        u32 read_len = read_end - current + 1;

        u8 *buf = malloc(read_len);
        if (!buf) {
            fmsh_print("Malloc failed\r\n");
            f_close(&fp);
            return FMSH_FAILURE;
        }

        if (mw_eeprom_read(current, buf, read_len) != 0) {
            fmsh_print("EEPROM read failed at 0x%08X\r\n", current);
            free(buf);
            f_close(&fp);
            return FMSH_FAILURE;
        }

   
        ret = f_write(&fp, buf, read_len, &bytes_written);
        if (ret != FMSH_SUCCESS || read_len != bytes_written)
        {
            fmsh_print("Error: Write failed, code: %d, Expected: %llu Bytes, Actual: %d Bytes\r\n", ret, read_len, bytes_written);
        }
        free(buf);

        current = read_end + 1;
    }

    f_close(&fp);
    fmsh_print("Success: data saved to %s\r\n", cnf_file);
    return FMSH_SUCCESS;
}


int mw_eeprom_write_bin(u32 start_addr, u32 end_addr, const char *cnf_file)
{
    int ret = FMSH_SUCCESS;
    FIL fp;

    ret = f_open(&fp, cnf_file, FA_READ);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Can't open binary file: %s\r\n", cnf_file);
        return FMSH_FAILURE;
    }

    u32 current = start_addr;

    u32 total_write_len = end_addr - start_addr + 1;
    u32 total_read_len = 0;

    while (current <= end_addr) 
    {
        u32 read_len = 0;
        u32 page_end = (current & ~(CONFIG_EEPROM_PAGE_SIZE - 1)) + CONFIG_EEPROM_PAGE_SIZE - 1;
        u32 write_end = (end_addr < page_end) ? end_addr : page_end;
        u32 write_len = write_end - current + 1;

        u8 *buf = (u8 *)malloc(write_len);
        if (!buf) {
            fmsh_print("malloc failed for write buffer\r\n");
            f_close(&fp);
            return FMSH_FAILURE;
        }

        ret = f_read(&fp, buf, write_len, &read_len);

        if (ret != FR_OK)
        {
            fmsh_print("Fail to read file, errNum: %d\r\n", ret);
            f_close(&fp);
            return FMSH_FAILURE;
        }

        total_read_len += read_len;

        if (read_len != write_len) {
            fmsh_print("File data insufficient! Need %zu bytes, read %zu bytes at 0x%08X\r\n",
                       write_len, read_len, current);
            free(buf);
            f_close(&fp);
            return FMSH_FAILURE;
        }


        if (mw_eeprom_write(current, buf, write_len) != 0) {
            fmsh_print("EEPROM write failed at 0x%08X\r\n", current);
            free(buf);
            f_close(&fp);
            return FMSH_FAILURE;
        }

        free(buf);
        current = write_end + 1;
    }


    if (total_read_len != total_write_len) {
        fmsh_print("Write length mismatch! Total need %zu bytes, read %zu bytes\r\n",
                   total_write_len, total_read_len);
        f_close(&fp);
        return FMSH_FAILURE;
    }

    f_close(&fp);
    fmsh_print("Success: data written to EEPROM (0x%08X-0x%08X) from %s\r\n",
               start_addr, end_addr, cnf_file);
    return FMSH_SUCCESS;
}


int mw_eeprom_read(u32 addr, u8 *data, int lenth)
{
    int size;

    if (lenth <= 0) 
    {
        fmsh_print("ERR: invalid length: %d\r\n", lenth);
        return FMSH_FAILURE;
    }

    u32 end_addr = addr + (u32)lenth - 1;

    if (addr >= CONFIG_EEPROM_EEPROM_SIZE || end_addr >= CONFIG_EEPROM_EEPROM_SIZE)
    {
        fmsh_print("ERR: out of range! addr:0x%x, len:%d, max_addr:0x%x\r\n", 
                   addr, lenth, CONFIG_EEPROM_EEPROM_SIZE - 1);
        return FMSH_FAILURE;
    }

    size = addr + lenth;
    while( addr < size )
    {
        u32 len = 0;
        len = size - addr;
        fmsh_print("Len = %d, [addr = 0x%x]\r\n", len, addr);

        len = (len > (CONFIG_EEPROM_PAGE_SIZE - (addr % CONFIG_EEPROM_PAGE_SIZE))) ? (CONFIG_EEPROM_PAGE_SIZE - (addr % CONFIG_EEPROM_PAGE_SIZE)) : (size - addr);

        fmsh_print("Len = 0x%x\r\n", len);

        if( eeprom_read_bytes(addr, CONFIG_EEPROM_ADDR_LEN, data, len) != FMSH_SUCCESS )
        {
            fmsh_print("ERR: eeprom_i2c read fail\r\n");
            return FMSH_FAILURE;
        }

        data += len;
        addr += len;
        
        delay_ms(10);
    } 
    return FMSH_SUCCESS;
}

int mw_eeprom_write(u32 addr, u8 *data, int lenth)
{
    int size;
    if (lenth <= 0) 
    {
        fmsh_print("ERR: invalid length: %d\r\n", lenth);
        return FMSH_FAILURE;
    }

    u32 end_addr = addr + (u32)lenth - 1;

    if (addr >= CONFIG_EEPROM_EEPROM_SIZE || end_addr >= CONFIG_EEPROM_EEPROM_SIZE)
    {
        fmsh_print("ERR: out of range! addr:0x%x, len:%d, max_addr:0x%x\r\n", 
                   addr, lenth, CONFIG_EEPROM_EEPROM_SIZE - 1);
        return FMSH_FAILURE;
    }
    fmsh_print("Lenth = %d, [0x%x]=0x%x\r\n",lenth, addr, *data);
    size = addr + lenth;
    while( addr < size )
    {
        
        u32 len = 0;

        len = size - addr;

        len = (len > (CONFIG_EEPROM_PAGE_SIZE - (addr % CONFIG_EEPROM_PAGE_SIZE))) ? (CONFIG_EEPROM_PAGE_SIZE - (addr % CONFIG_EEPROM_PAGE_SIZE)) : (size - addr);

        if( eeprom_write_bytes(addr,CONFIG_EEPROM_ADDR_LEN, data, len) != FMSH_SUCCESS )
        {
            fmsh_print("ERR: eeprom_i2c write fail\r\n");
            return FMSH_FAILURE;
        }

        data += len;
        addr += len;
        delay_ms(10);
    } 

    return FMSH_SUCCESS;
}
