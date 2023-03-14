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
2. Transfer the file to the Raspberry Pi Pico: `./deploy.sh {name of app}`.
    * Ex. ```./deploy.sh App-Template```
3. However, most of the time, you will want to build it first, which you can do with the ```-b``` flag:
    * ```./deploy.sh -b App-Template```

## Project Structure

```
/RP2040-FreeRTOS
|
|___/fsw                    // The main folder/application that conatins the flight software
|   |___CMakeLists.txt      // Application-level CMake config file
|   |___include             // All of the header files for the project
|   |___src                 // All of the .c files for the project
|   |___.vscode             // vscode settings for the project in case you open the project with the fsw folder
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

## The Apps

This repo includes a number of deployable apps. The project builds them all, sequentially. Exclude apps from the build process by commenting out their `add_subdirectory()` lines in the top-level `CMakeLists.txt`.

### App One: Template

This C app provides a simple flip-flop using an on-board LED and an LED wired between GPIO 20 and GND. The board LED flashes every 500ms under one task. When its state changes, a message containing its state is added to a FreeRTOS inter-task xQueue. A second task checks for an enqueued message: if one is present, it reads the message and sets the LED it controls — the GPIO LED — accordingly to the inverse of the board LED’s state.

The code demonstrates a basic FreeRTOS setup, but you can replace it entirely with your own code if you’re using this repo’s contents as a template for your own projects.

### App Two: Scheduling

This C++ app builds on the first by adding an MCP9808 temperature sensor and an HT16K33-based LED display. It is used in [this blog post](https://blog.smittytone.net/2022/03/04/further-fun-with-freertos-scheduling/).

### App Three: IRQs

This C++ app builds on the second by using the MCP9808 temperature sensor to trigger an interrupt. It is used in [this blog post](https://blog.smittytone.net/2022/03/20/fun-with-freertos-and-pi-pico-interrupts-semaphores-notifications/).

### App Four: Timers

This C++ app provides an introduction to FreeRTOS’ software timers. No extra hardware is required. It is used in [this blog post](https://blog.smittytone.net/2022/06/14/fun-with-freertos-and-the-pi-pico-timers/).
