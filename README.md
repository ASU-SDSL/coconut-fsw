# Coconut Flight Software

This repo contains the flight software for the SDSL Coconut CubeSat. It uses FreeRTOS on the Raspberry Pi Pico. This repo is based on [this repo](https://github.com/smittytone/RP2040-FreeRTOS) so check it out for reference.

## Instalation Instructions

1. Setup GitHub SSH and install git - https://docs.github.com/en/authentication/connecting-to-github-with-ssh
2. In a directory of your choice, run the following command (it may take awhile to install all of the libraries) - ```git clone --recurse-submodules git@github.com:ASU-SDSL/coconut-fsw.git```
3. To map the pico-sdk directory to the appropriate envirnment variable, add this line to your ~/.bashrc file (note: the .bashrc file only runs when you login, so you will have to run this command in your command line the first time):
    * ```export PICO_SDK_PATH="$HOME/Documents/coconut/coconut-fsw/lib/pico-sdk"```
4. Install extra tools: ```sudo apt install cmake gcc-arm-none-eabi build-essential```

## Usage

1. Connect your device (RPI Pico) so it’s ready for file transfer.
2. Build the project and transfer it to the Raspberry Pi Pico: `./deploy.sh`.
    * Just build the project: `./deploy.sh -b`
    * Just flash the project to the pico: `./deploy.sh -f`

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
|___/lib                    // includes all of the libraries/submodules needed for the project
|   |___ ... 
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
