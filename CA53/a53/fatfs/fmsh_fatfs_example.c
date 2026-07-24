#include <stdlib.h>
#include "ff.h"
#include "fmsh_common.h"
#include "fmsh_fatfs_example.h"





static void printCode(char *name, unsigned char* addr, unsigned int size)
{
    unsigned int i;
    fmsh_print("%s Hex:",name);
    for(i=0;i<size;i++)
    {
        if((i % 16) == 0)
            fmsh_print("\n%02x: ",i);
        fmsh_print("%02x ",addr[i]);
    }
    fmsh_print("\n");

}

int wr_test_example(char *host_path, char *dst_path)
{
    FATFS fs1; 
    FIL filr, filw; 
    u32 Status = FMSH_SUCCESS;
    u32 bin_size = 0;
    u32 ulbw = 0;
    int ret = FMSH_SUCCESS;
    char *prbuf, *pwbuf;
    unsigned char *prbuf_AlignStart;
    unsigned char *pwbuf_AlignStart;
    u32 cahcelinesize = 64;


    Fmsh_DCacheDisable();
        
    ret = f_open(&filr, host_path, FA_READ);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Fail to open file, errNum: %d\r\n", ret);
        return FMSH_FAILURE;
    }

    prbuf = malloc(FWRITE_READ_BUFFER_SIZE_MAX + 2 * cahcelinesize);
    if (NULL == prbuf)
    {
        fmsh_print("prbuf malloc err\r\n");
        return FMSH_FAILURE;
    }

    pwbuf = malloc(FWRITE_READ_BUFFER_SIZE_MAX + 2 * cahcelinesize);
    if (NULL == pwbuf)
    {
        fmsh_print("pwbuf malloc err\r\n");
        free(prbuf);
        return FMSH_FAILURE;
    }
    prbuf_AlignStart = (char *)(((long long)prbuf + cahcelinesize) &
                                (~((long long)cahcelinesize - 1)));
    pwbuf_AlignStart = (char *)(((long long)pwbuf + cahcelinesize) &
                                 (~((long long)cahcelinesize - 1)));
    ret = f_read(&filr, (void *)prbuf_AlignStart, f_size(&filr), &bin_size); 
    if (ret != FR_OK)
    {
        fmsh_print("Fail to read file, errNum: %d\r\n", ret);
        f_close(&filr);
        free(pwbuf);
        free(prbuf);
        return FMSH_FAILURE;
    }

    Fmsh_DCacheFlushRange((uintptr_t)prbuf_AlignStart, bin_size);

    printCode(host_path, (unsigned char*)prbuf_AlignStart, 256);

    ret = f_open(&filw, dst_path, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
    if (ret)
    {
        fmsh_print("Unable to open file %s: %d\r\n", dst_path, ret);
        f_close(&filr);
        free(pwbuf);
        free(prbuf);
        return FMSH_FAILURE;
    }
    ret = f_write(&filw, (void *)prbuf_AlignStart, bin_size, &ulbw);

    if (ret || (bin_size != ulbw))
    {
        fmsh_print("Write File To EMMC Failed[%d].\r\n", ret);
        f_close(&filr);
        f_close(&filw);
        free(pwbuf);
        free(prbuf);
        return FMSH_FAILURE;
    }
    
     f_close(&filw);

     ret = f_open(&filw, dst_path, FA_READ);
     if (ret)
     {
        fmsh_print("Unable to open file %s: %d\r\n", dst_path, ret);
        f_close(&filr);
        free(pwbuf);
        free(prbuf);
        return FMSH_FAILURE;
     }

    ret = f_read(&filw, (void *)pwbuf_AlignStart, f_size(&filw), &bin_size); 
    if (ret != FR_OK)
    {
        fmsh_print("Fail to read file, errNum: %d\r\n", ret);
        f_close(&filr);
        f_close(&filw);
        free(pwbuf);
        free(prbuf);
        return FMSH_FAILURE;
    }

    Fmsh_DCacheFlushRange((uintptr_t)pwbuf_AlignStart, bin_size);

    if (0 != memcmp(prbuf_AlignStart, pwbuf_AlignStart, bin_size))
    {
        fmsh_print("Readback check err\r\n");
        f_close(&filr);
        f_close(&filw);
        free(pwbuf);
        free(prbuf);
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("Readback check OK\r\n");
    }

    printCode(host_path, (unsigned char*)prbuf_AlignStart, 256);
    printCode(dst_path, (unsigned char*)pwbuf_AlignStart, 256);

    f_close(&filr);
    f_close(&filw);
    free(pwbuf);
    free(prbuf);
    Fmsh_DCacheEnable();
    return Status;
}