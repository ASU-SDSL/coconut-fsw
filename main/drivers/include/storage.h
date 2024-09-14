#include <stdbool.h>

#include "FreeRTOS.h"

#include "ff.h"
#include "log.h"

#define READ_BUFFER_SIZE 256

void sd_write(const char* fileName, const char* text, int append_flag);
void sd_read(const char* fileName, char* result_buffer);
void sd_delete(const char* fileName);
