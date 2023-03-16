# RP2040-FreeRTOS Template 1.4.1

This repo contains the flight software for the Coconut CubeSat. It uses FreeRTOS on the Raspberry Pi Pico. This repo is based on [this repo](https://github.com/smittytone/RP2040-FreeRTOS) so check it out for reference.

## Instalation Instructions

These tools must be installed to compile and upload the code in this repo. Follow these steps:
1. In a directory of your choice, run - ```git clone -b master --recurse-submodules https://github.com/raspberrypi/pico-sdk.git```
2. In a directory of your choice, run - ```git clone https://github.com/FreeRTOS/FreeRTOS-Kernel```
3. To map these directories to the appropriate envirnment variables, add these lines to your ```~/.bashrc``` file (note: the .bashrc file only runs when you login, so you will have to run these commands in your command line the first time):
    * ```export PICO_SDK_PATH="$HOME/Documents/coconut/freertos/pico-sdk"```
    * ```export FREERTOS_KERNEL_PATH="$HOME/Documents/coconut/freertos/FreeRTOS-Kernel"```
4. Clone this repository - ```git clone https://github.com/ASU-SDSL/coconut-fsw```
5. Install extra tools: ```sudo apt install cmake gcc-arm-none-eabi build-essential```

## Usage

1. Connect your device so it’s ready for file transfer.
2. Build the project and transfer it to the Raspberry Pi Pico: `./deploy.sh -b -u`.
    * Just build the project: `./deploy.sh -b`
    * Just transfer the project: `./deploy.sh -u`

NOTE: if you get an error that the directory does not exist, change the `rpi_path` variable in the `deploy.sh` to your RPI's mounted file location.

## Project Structure

```
/RP2040-FreeRTOS
|
|___/main                   // The main folder/application that conatins the flight software
|   |___CMakeLists.txt      // Application-level CMake config file
|   |___include             // All of the header files for the project
|   |___src                 // All of the .c files for the project
|   |___.vscode             // vscode settings for the project in case you open the project with the main folder
|   
|___/Common                 // Source code common to applications 2-4 (C++)
|
|___/Config
|   |___FreeRTOSConfig.h    // FreeRTOS project config file
|
|___CMakeLists.txt          // Top-level project CMake config file
|___pico_sdk_import.cmake   // Raspberry Pi Pico SDK CMake import script
|___deploy.sh               // Build-and-deploy shell script
|
|___rp2040.code-workspace   // Visual Studio Code workspace
|___rp2040.xcworkspace      // Xcode workspace - I don't think we really need this
|
|___README.md
```
