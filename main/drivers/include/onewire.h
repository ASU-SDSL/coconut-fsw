// header file for one wire device
#pragma once

#include <byteswap.h>
#include <OneWire.h>

#define SERIAL_NUM 64
#define ROM_CODE 64
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD 0x4E
#define COPY_SCRATCHPAD 0x48
#define CONVERT 0x44
#define RECALL 0xB8
#define READ_POWER 0xB4
#define COMMUNICATION_PIN 0 //add number later
