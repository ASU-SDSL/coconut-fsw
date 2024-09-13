#pragma once

#include <stdint.h>
#include <stdio.h>
#include "diskio.h"
#include "ff.h"
#include "FreeRTOS.h"

static DVTATUS gStatus = STA_NOINIT;

static BYTE *disk = NULL;

#define NUM_SECTORS 128 

DSTATUS disk_initialize(
    BYTE pdrv
);

DSTATUS disk_status(
    BYTE drv
);

DRESULT disk_read(
    BYTE pdrv,
    BYTE *buff,
    LBA_t sector,
    UINT count
);

DRESULT disk_write(
    BYTE pdrv,
    const BYTE *buff,
    LBA_t sector,
    UINT count
);

DRESULT disk_ioctl (
    BYTE pdrv,     /* [IN] Drive number */
    BYTE cmd,      /* [IN] Control command code */
    void* buff     /* [I/O] Parameter and data buffer */
);

DWORD get_fattime(
    void
);
