/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "fmsh_sdhci_card.h"
#include "fmsh_rtc_public.h"

/* Definitions of physical drive number for each drive */
#define DEV_MMC0	0	/* Example: Map MMC/SD0 card to physical drive 0 */
#define DEV_MMC1	1	/* Example: Map MMC/SD1 card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

static FSdPsu_T *sd0Ptr, *sd1Ptr;
extern FRtcPs_T g_RTC;
static DWORD cur_time[6] = {2026, 1, 18, 0, 0, 0};

/*-----------------------------------------------------------------------*/
/* 31-25: Year(0-127, org. 1980)                                         */
/* 24-21: Month(1-12)                                                    */ 
/* 20-16: Day(1-31)                                                      */
/* 15-11: Hour(0-23)                                                     */
/* 10-05: Minute(0-59)                                                   */ 
/* 04-00: Second(0-29*2)                                                 */ 
/*-----------------------------------------------------------------------*/
DWORD get_fattime()
{
    int ret;
    DWORD time;
    rtc_time tm;
    
    if(g_RTC.base_address != NULL) {
        ret = FRtcPs_read_time(&g_RTC,  &tm);
    } 
    else {
        ret = 1;
    }
    
    if ( ret || tm.tm_year == 70) 
    {
        fmsh_print_warning("Warning: RTC time error, use default value!\r\n");
        (void)memset(&tm, 0, sizeof(tm));
        time = ((tm.tm_year + cur_time[0] - 1980) & 0x7f) << 25;
        time |= ((tm.tm_mon + cur_time[1]) & 0xf) << 21;
        time |= ((tm.tm_mday + cur_time[2]) & 0x1f) << 16;
        time |= ((tm.tm_hour + cur_time[3]) & 0x1f) << 11;
        time |= ((tm.tm_min + cur_time[4]) & 0x3f) << 5;
        time |= (((tm.tm_sec + cur_time[5])/2) & 0x1f);
        return time;
    }

    time = ((tm.tm_year - 80) & 0x7f) << 25;
    time |= ((tm.tm_mon + 1) & 0xf) << 21;
    time |= (tm.tm_mday & 0x1f) << 16;
    time |= (tm.tm_hour & 0x1f) << 11;
    time |= (tm.tm_min & 0x3f) << 5;
    time |= ((tm.tm_sec/2 ) & 0x1f);
        
    return time;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS MMC_disk_status(int device_id)
{
    int ret;

    FSdPsu_T *sdPtr = NULL;
    
    if(device_id == 0) {
        sdPtr = sd0Ptr;
     }   
    else if(device_id == 1) {
        sdPtr = sd1Ptr;
    }
    else{
        ;/* no deal with */
    }
    if( (sdPtr == NULL) || (sdPtr->is_inited == 0) ){
        return STA_NOINIT;
    }
    
    //check if card present and write prot
    ret = FSdPsu_Host_CardDetect(sdPtr);
    if(ret) {
        return STA_NODISK;
    }
    
    //read wp pin level form sd card
    ret = FSdPsu_Host_WriteProt(sdPtr);
    if(ret == 1) {
        return STA_PROTECT;
    }

    return 0;
}

DSTATUS USB_disk_status()
{
    return 0;
}

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	DSTATUS result;

	switch (pdrv) {
	case DEV_MMC0 :
		result = MMC_disk_status(0);

		// translate the reslut code here
        stat = result;
                
		return stat;

	case DEV_MMC1 :
		result = MMC_disk_status(1);

		// translate the reslut code here
        stat = result;
                
		return stat;

	case DEV_USB :
		result = USB_disk_status();

		// translate the reslut code here
        stat = result;
                
		return stat;
        default:
                break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
int MMC_disk_initialize(int device_id)
{
    int ret;
    FSdPsu_Config_T *configPtr;
    FSdPsu_T *sdPtr = NULL;
    
    //get FSdPsu_Config_T
    configPtr = FSdPsu_LookupConfig(device_id);
    if(configPtr == NULL)
        return STA_NOINIT;
    
    //alloc space for sdPtr
    if(device_id == 0) {
        sd0Ptr = malloc(sizeof(FSdPsu_T));
        sdPtr = sd0Ptr;
     }   
    else if(device_id == 1) {
        sd1Ptr = malloc(sizeof(FSdPsu_T));
        sdPtr = sd1Ptr;
    }
    else{
        ;/* no deal with */
    }
    if(sdPtr == NULL) {
        return STA_NOINIT;
    }
    
    //initialize sdPtr
    ret = FSdPsu_CfgInitialize(sdPtr, configPtr);
    if(ret) {
        return STA_NOINIT;
    }

    ret = FSdPsu_CardInit(sdPtr, NULL);  
    if(ret) {
        return STA_NOINIT;
    }
    
    ret = FSdPsu_Host_WriteProt(sdPtr);
    if(ret == 1) {
        return STA_PROTECT;
    }
            
    return 0;    
}

int USB_disk_initialize()
{
    return 0;
}

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_MMC0 :
		result = MMC_disk_initialize(0);

		// translate the reslut code here
                stat = result;
                
		return stat;

	case DEV_MMC1 :
		result = MMC_disk_initialize(1);

		// translate the reslut code here
                stat = result;
                
		return stat;

	case DEV_USB :
		result = USB_disk_initialize();

		// translate the reslut code here
                 stat = result;
                 
		return stat;
         default:
                break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT MMC_disk_read(int device_id, const BYTE *buff, LBA_t sector, UINT count)
{
    int ret;
    FSdPsu_T *sdPtr = NULL;
    
    if(device_id == 0) {
        sdPtr = sd0Ptr;
     }   
    else if(device_id == 1) {
        sdPtr = sd1Ptr;
    }
    else{
        ;/* no deal with */
    }
    if(sdPtr == NULL) {
        return RES_PARERR;
    }
    
    ret = FSdPsu_Bread(sdPtr, sector, count, (unsigned char*)buff);
    if(ret != count) {
        return RES_ERROR;
    }
    
    return RES_OK;
}

DRESULT USB_disk_read(const BYTE *buff, LBA_t sector, UINT count)
{
    return RES_OK;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	DRESULT result;

	switch (pdrv) {
	case DEV_MMC0 :
		// translate the arguments here

		result = MMC_disk_read(0, buff, sector, count);

		// translate the reslut code here
        res = result;
                
		return res;

	case DEV_MMC1 :
		// translate the arguments here

		result = MMC_disk_read(1, buff, sector, count);

		// translate the reslut code here
        res = (DRESULT)result;
                
		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here
        res = (DRESULT)result;
                 
		return res;
        default:
          break;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT MMC_disk_write(int device_id, const BYTE *buff, LBA_t sector, UINT count)
{
    int ret;
    FSdPsu_T *sdPtr = NULL;
    
    if(device_id == 0) {
        sdPtr = sd0Ptr;
     }   
    else if(device_id == 1) {
        sdPtr = sd1Ptr;
    }
    else{
        ;/* no deal with */
    }
    if(sdPtr == NULL) {
        return RES_PARERR;
    }
    
    ret = FSdPsu_Bwrite(sdPtr, sector, count, (unsigned char*)buff);
    if(ret != count) {
        return RES_ERROR;
    }
    
    return RES_OK;
}

DRESULT USB_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    return RES_OK;
}

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	DRESULT result;

	switch (pdrv) {
	case DEV_MMC0 :
		// translate the arguments here

		result = MMC_disk_write(0, buff, sector, count);

		// translate the reslut code here
        res = result;
                
		return res;

	case DEV_MMC1 :
		// translate the arguments here

		result = MMC_disk_write(1, buff, sector, count);

		// translate the reslut code here
        res = result;
                
		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here
        res = result;
                
		return res;
        default:
                break;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT MMC_disk_ioctl(int device_id, BYTE cmd, void *buff)
{
    DRESULT ret;
    FSdPsu_T *sdPtr = NULL;
    struct sdmmc_card *card; 
    
    if(device_id == 0) {
        sdPtr = sd0Ptr;
     }   
    else if(device_id == 1) {
        sdPtr = sd1Ptr;
    }
    else{
        ;/* no deal with */
    }
    if(sdPtr == NULL) {
        return RES_PARERR;
    }
    
    card = &(sdPtr->card[0]);
    
    switch (cmd) {
    case CTRL_SYNC :	/* Make sure that no pending write process */
        ret = RES_OK;
        break;
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
        *(DWORD *)buff = card->block_max;
        ret = RES_OK;
        break; 
    case GET_SECTOR_SIZE : /* Get sector size in bytes (DWORD) */
        *(DWORD *)buff = SDMMC_MAX_BLOCK_LEN;
        ret = RES_OK;
        break;
    case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
        *(DWORD *)buff = 0x1000;
        ret = RES_OK;
        break;
    default:
        ret = RES_PARERR;
        break;
    }
    
    return ret;
}

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	DRESULT result;

	switch (pdrv) {
	case DEV_MMC0 :

		// Process of the command for the RAM drive
        result = MMC_disk_ioctl(0, cmd, buff);
        res = result;    
		return res;

	case DEV_MMC1 :

		// Process of the command for the MMC/SD card
        result = MMC_disk_ioctl(1, cmd, buff);
        res = result;    
		return res;

	case DEV_USB :

		// Process of the command the USB drive
        res = RES_OK;
		return res;
        default:
          break;
	}

	return RES_PARERR;
}

