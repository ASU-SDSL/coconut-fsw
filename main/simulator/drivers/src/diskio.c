#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ff.h"
#include "FreeRTOS.h"
#include "diskio.h"

static DSTATUS gStatus = STA_NOINIT;

static BYTE *disk = NULL;

#define NUM_SECTORS 128 

// Inspired by https://stackoverflow.com/questions/77411723/how-do-i-use-a-ram-buffer-for-a-fatfs-filesystem

DSTATUS disk_initialize(
    BYTE pdrv
)
{
    if (pdrv == 0)
    {
        if (disk == NULL)
        {
            disk = pvPortMalloc(FF_MIN_SS * NUM_SECTORS);
            if (disk != NULL)
            {
                // for RAM buffer, no additional setup needed, so clear NOINIT bit.
                gStatus &= ~STA_NOINIT;
                // set malloc'd buffer to zeros
                memset(disk, 0, FF_MIN_SS * NUM_SECTORS);
            }
            else
            {
                gStatus = STA_NOINIT;
            }
        }
    }

    return gStatus;
}

DSTATUS disk_status(
    BYTE drv
)
{
    return gStatus;
}

DRESULT disk_read(
    BYTE pdrv,
    BYTE *buff,
    LBA_t sector,
    UINT count
)
{
    DRESULT res = RES_OK;
    if (pdrv == 0 && sector <= NUM_SECTORS)
    {
        memcpy(buff, disk + (sector * FF_MIN_SS), count * FF_MIN_SS);
    }
    else
    {
        res = RES_PARERR;
    }

    return res;
}

DRESULT disk_write(
    BYTE pdrv,
    const BYTE *buff,
    LBA_t sector,
    UINT count
)
{
    DRESULT res = RES_OK;

    if (pdrv == 0 && sector <= NUM_SECTORS)
    {
        memcpy(disk + (sector * FF_MIN_SS), buff, count * FF_MIN_SS);
    }
    else
    {
        res = RES_PARERR;
    }

    return res;
}

DRESULT disk_ioctl (
    BYTE pdrv,     /* [IN] Drive number */
    BYTE cmd,      /* [IN] Control command code */
    void* buff     /* [I/O] Parameter and data buffer */
)
{
    DRESULT res = RES_OK;
    if (pdrv != 0)
    {
        res = RES_PARERR;
    }
    else
    {
        switch(cmd)
        {
            case GET_SECTOR_COUNT:
                *(DWORD *)buff = NUM_SECTORS;
                res = RES_OK;
                break;
            case GET_BLOCK_SIZE:
                *(DWORD *)buff = 128;
                res = RES_OK;
                break;
            case CTRL_SYNC:
                res = RES_OK;
                break;
            default:
                res = RES_PARERR;
                break;
        }
    }

    return res;
}

DWORD get_fattime(
    void
)
{
    // 0 will make the file timestamp invalid, but I don't need an accurate timestamp.
    return 0;
}