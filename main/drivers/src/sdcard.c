#include "sdcard.h"
#include "ff.h"
#include "sd_card.h"

void write(const char* fileName, const char* text, int append_flag) {
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];


    // Wait for user to press 'enter' to continue
    printf("\r\nSD card test. Press 'enter' to start.\r\n");

    {
        // this is called implicitly now
        // Initialize SD card
        if (!sd_init_driver()) {
            printf("ERROR: Could not initialize SD card\r\n");
            while (true);
        }
        

        
        //sd_card_t* sd = sd_get_by_num(0);
        int file_open_flags = FA_WRITE | FA_CREATE_ALWAYS;
        if(append_flag) { file_open_flags = FA_OPEN_APPEND | FA_WRITE; }

        // Mount drive
        fr = f_mount(&fs, "0:", 1);
        if (fr != FR_OK) {
            printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
            while (true);
        }

        // Open file for writing ()
        fr = f_open(&fil, fileName, file_open_flags);
        if (fr != FR_OK) {
            printf("ERROR: Could not open file (%d)\r\n", fr);
            while (true);
        }

        // Write something to file
        ret = f_printf(&fil, text);
        if (ret < 0) {
            printf("ERROR: Could not write to file (%d)\r\n", ret);
            f_close(&fil);
            while (true);
        }

        // Close file
        fr = f_close(&fil);
        if (fr != FR_OK) {
            printf("ERROR: Could not close file (%d)\r\n", fr);
            while (true);
        }



        // Unmount drive
        f_unmount("0:");

        // Loop forever doing nothing
        while (true) {
            sleep_ms(1000);
        }
    }
}

void read(const char* fileName) {
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char readBuffer[255];

    // mount filesystem
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }

    // open file
    fr = f_open(&fil, fileName, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }    


    // read from file, buffer only allows 255 chars
    // might need to switch the last argument to uint* rather than null
    fr = f_read(&fil, readBuffer, 255, NULL);
    if(fr != FR_OK) {
        printf("ERROR: Could not read from file (%d)\r\n", fr);
        while (true);
    }

    // close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // unmount fs
    f_unmount("0:");
}

void delete(const char * fileName) {
    FRESULT fr;
    FATFS fs;

    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }

    fr = f_unlink(fileName);
    if (fr != FR_OK) {
        printf("ERROR: Could not remove file (%d)\r\n", fr);
        while (true);
    }

    f_unmount("0:");
}
