/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		1	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
// custom code for fatfs
#include "stm32h7xx_hal.h"
#include "custom/debug.h"

#include "custom/sd_driver.h"

extern SD_HandleTypeDef hsd1;
extern HAL_SD_CardInfoTypeDef CL_pCardInfo;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case DEV_RAM :
		// translate the reslut code here
		return STA_NODISK;

	case DEV_MMC :
		if (CL_getSDState() == HAL_OK){
			return RES_OK;
		} else {
			return RES_ERROR;
		}

	case DEV_USB :
		// translate the reslut code here
		return STA_NODISK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case DEV_RAM :
		return RES_ERROR;
	case DEV_MMC :
		// translate the reslut code here
		if (CL_readSdInfo() == HAL_OK){
			return RES_OK;
		} else {
			return RES_ERROR;
		}

	case DEV_USB :
		return RES_ERROR;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
	case DEV_RAM :
		return RES_ERROR;
	case DEV_MMC :
		// translate the arguments here
		if (CL_sdReadBlocks(buff, sector, count) == HAL_OK) {
			return RES_OK;
		} else {
			return RES_ERROR;
		}

	case DEV_USB :
		return RES_ERROR;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here
		return RES_ERROR;

	case DEV_MMC :
        if (CL_sdWriteBlocks(buff, sector, count) == HAL_OK) {
        	return RES_OK;
        } else {
        	return RES_ERROR;
        }

	case DEV_USB :
		return RES_ERROR;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_ERROR;

	switch (pdrv) {
	case DEV_RAM :
		return RES_PARERR;

	case DEV_MMC :

		// Process of the command for the MMC/SD card
		switch (cmd){
			/* Make sure that no pending write process */
			case CTRL_SYNC :
				res = RES_OK;
				break;

			/* Get number of sectors on the disk (DWORD) */
			case GET_SECTOR_COUNT :
				*(DWORD*)buff = CL_pCardInfo.LogBlockNbr;
				print("sec count: %d\n", CL_pCardInfo.LogBlockNbr);
				res = RES_OK;
				break;

			/* Get R/W sector size (WORD) */
			case GET_SECTOR_SIZE :
				*(WORD*)buff = CL_pCardInfo.LogBlockSize;
				res = RES_OK;
				break;

			/* Get erase block size in unit of sector (DWORD) */
			case GET_BLOCK_SIZE :
				*(DWORD*)buff = CL_pCardInfo.BlockSize;
				res = RES_OK;
				break;

			default:
				res = RES_PARERR;
		}
		return res;

	case DEV_USB :
		return RES_PARERR;
	}

	return RES_PARERR;
}

