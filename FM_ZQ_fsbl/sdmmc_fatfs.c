
#include <string.h>

#include "boot_main.h"
#include "ff.h"
#include "fmsh_common.h"

static FIL fil; /* File object */
static FATFS fatfs;

int InitSD (char *filename)
{
    FRESULT rc;

    // open file
    fmsh_print_info("Open file %s!\r\n", filename);
    rc = f_open(&fil, filename, FA_READ);
    if (rc != FR_OK)
    {
        fmsh_print_err("Failed to open file, rc = %d\r\n", rc);
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int ReleaseSD (void)
{
    FRESULT rc;

    fmsh_print_info("Close file!\r\n");

    rc = f_close(&fil);
    if (rc)
    {
        fmsh_print_err("Failed to close file!\r\n");
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int SDWrite (u32 offset, void *buf, int len)
{
    FRESULT rc; /* Result code*/
    UINT bw;

    rc = f_lseek(&fil, offset);
    if (rc)
    {
        fmsh_print_err("Failed to lseek.\r\n");
        return FMSH_FAILURE;
    }

    rc = f_write(&fil, buf, len, &bw);
    if (rc)
    {
        fmsh_print_err("Failed to write file, rc = 0x%x\r\n", rc);
        return FMSH_FAILURE;
    }

    rc = f_sync(&fil);
    if (rc)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int SDRead (u32 offset, void *buf, int len)
{
    FRESULT rc; /* Result code*/
    UINT br;

    rc = f_lseek(&fil, offset);
    if (rc)
    {
        fmsh_print_err("Failed to lseek.\r\n");
        return FMSH_FAILURE;
    }

    rc = f_read(&fil, buf, len, &br);
    if (rc)
    {
        fmsh_print_err("Failed to read file, rc = 0x%x\r\n", rc);
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

u32 FmshFsbl_SdAccess (u32 SrcAddress, u32 DestAddress, u32 Length)
{
    u32 Status = FMSH_SUCCESS;
    Status = SDRead(SrcAddress, (u32 *)DestAddress, Length);

    return Status;
}

u32 FmshFsbl_InitSd (u32 DeviceFlags)
{
    char buffer[32U] = {0U};
    char *boot_file = buffer;
    u32 Status = FMSH_SUCCESS;
    u32 MultiBootReg;
    u32 FileNameLen = 0;
    u32 Index = 0;
    u32 Value = 0;

    FRESULT rc;

    if (DeviceFlags == FSBL_SD_DRV_NUM_0)
    {
        rc = f_mount(&fatfs, "0:/", 1);
    }
    else
    {
        rc = f_mount(&fatfs, "1:/", 1);
    }

    if (rc == FR_NO_FILESYSTEM)
    {
        fmsh_print_err("No filesystem exist!\r\n");
        return FMSH_FAILURE;
    }
    else if (rc != FR_OK)
    {
        fmsh_print_err("Failed to mount filesystem, rc=%d\r\n", rc);
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print_info("Mount filesystem succeed!\r\n");
    }

    MultiBootReg = ReadReg(SAC_MULTI_BOOT_REG);

    if (0x0U == MultiBootReg)
    {
        /* SD file name is BOOT.BIN when Multiboot register value is 0 */
        if (DeviceFlags == FSBL_SD_DRV_NUM_0)
        {
            (void)memcpy(boot_file, "BOOT.BIN", FSBL_BASE_FILE_NAME_LEN_SD_0);
        }
        else
        {
            /* For second SD instance, include drive number 1 as well */
            (void)memcpy(boot_file, "1:/BOOT.BIN", FSBL_BASE_FILE_NAME_LEN_SD_1);
        }
    }
    else
    {
        /* set default SD file name as BOOT0000.BIN */
        if (DeviceFlags == FSBL_SD_DRV_NUM_0)
        {
            (void)memcpy(boot_file, "BOOT0000.BIN",
                   FSBL_BASE_FILE_NAME_LEN_SD_0 + FSBL_NUM_DIGITS_IN_FILE_NAME);
            FileNameLen = FSBL_BASE_FILE_NAME_LEN_SD_0;
        }
        else
        {
            /* For second SD instance, include drive number 1 as well */
            (void)memcpy(boot_file, "1:/BOOT0000.BIN",
                   FSBL_BASE_FILE_NAME_LEN_SD_1 + FSBL_NUM_DIGITS_IN_FILE_NAME);
            FileNameLen = FSBL_BASE_FILE_NAME_LEN_SD_1;
        }

        /* Update file name (to BOOTXXXX.BIN) based on Multiboot register value
         */
        for (Index = FileNameLen - 1U;
             Index >= (FileNameLen - FSBL_NUM_DIGITS_IN_FILE_NAME); Index--)
        {
            Value = MultiBootReg % 10U;
            MultiBootReg = MultiBootReg / 10U;
            boot_file[Index] += (char)Value;
            if (MultiBootReg == 0U)
            {
                break;
            }
        }
    }

    Status = InitSD(boot_file);

    return Status;
}
