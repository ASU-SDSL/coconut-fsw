#include <FreeRTOS.h>

void sd_write(const char* fileName, const char* text, int append_flag);
const char* sd_read(const char* fileName);
void sd_delete(const char* fileName);
