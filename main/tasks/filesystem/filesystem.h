/*
TODO:   sd card driver does not look finished, so finish the driver to read and write to sd card, plus any other qol functions
        write user functions. all internal functions and structs will come out of necessity.

*/


#pragma once

#include "sdcard.h"
#include <semphr.h>

//create global mutex for sd card
SemaphoreHandle_t sd_mutex;


/* USER FUNCTIONS */

//read file
char* read_file(const char* file_name);

//write file
// may want to add return value to specify whether operation was successful
void write_file(const char* file_name, const char* text_to_write);

//append file
void append_file(const char* file_name, const char* text_to_append);

//create file
void create_file(const char* new_file_name);

//delete file
void delete_file(const char* file_name);


/* INTERNAL FUNCTIONS */


// Main Task
void sd_task(void* unused_arg);