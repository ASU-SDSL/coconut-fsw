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
#include "mram.h"

/* Definitions of physical drive number for each drive */
#define DEV_MRAM		0 /* not sure what the drive # is, should be 0 */
#define DEVICE_SIZE 	4 * 1024 * 1024 /* 4Mb */
#define BLOCK_SIZE 	512U


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv) {
	DSTATUS disk_status = STA_NOINIT;

	// TODO: Update w/ global var
	switch (pdrv) {
	case DEV_MRAM :
		disk_status = ~STA_NOINIT;
		break;
	}

	return disk_status;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv) {
	DSTATUS initialization_status = STA_NOINIT;

	switch (pdrv) {
		case DEV_MRAM:
			initialize_mram();
			initialization_status = ~STA_NOINIT;
			break;
	}

	return initialization_status;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
/* pdrv - Physical drive nmuber to identify the drive */
/* buff - Data buffer to store read data, must be allocated by caller */
/* sector - Start sector in LBA */
/* count - Number of sectors to read */
DRESULT disk_read (BYTE pdrv, BYTE *buff, LBA_t sector, UINT count){
	int result;

	switch (pdrv) {
		case DEV_MRAM: {
			DWORD address = sector * BLOCK_SIZE;

			for(int i  = 0; i < count; i++) {
				int result = read_bytes(address, buff, BLOCK_SIZE);

				if(result != 0) {
					return RES_ERROR;
				}

				buff += BLOCK_SIZE;
				address += BLOCK_SIZE;
			}
			return RES_OK;
		}
		default:
			break;
	}
	
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* pdrv - Physical drive nmuber to identify the drive */
/* buff -  Data to be written */
/* sector - Start sector in LBA */
/* count - Number of sectors to write */
#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
	int result;

	switch (pdrv) {
		case DEV_MRAM: {
			DWORD address = sector * BLOCK_SIZE;
			for(int i = 0; i < count; i++) {
				int result = write_bytes(address, buff, BLOCK_SIZE);

				if(result != 0) {
					return RES_ERROR;
				}

				buff += BLOCK_SIZE;
				address += BLOCK_SIZE;
			}
			return RES_OK;
		}
		default:
			break;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
/* pdrv - Physical drive nmuber (0..) */
/* cmd - Control code */
/* buff - Buffer to send/receive control data */
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff) {
	DRESULT res;

	switch(cmd) {
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = DEVICE_SIZE / BLOCK_SIZE;
			break;
		case GET_SECTOR_SIZE:
			*(DWORD*)buff = BLOCK_SIZE;
    		res = RES_OK;
			break;
		case CTRL_SYNC:
    		res = RES_OK;
			break;
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = BLOCK_SIZE;
			res = RES_OK;
			break;
		default:
			res = RES_PARERR;
			break;
	} 

	return res;
}

DWORD get_fattime(void) {
	// TODO: add actual timestamp
    // 0 will make the file timestamp invalid, works but obviously not ideal
    return 0;
}