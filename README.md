# Coconut Flight Software

This repo contains the flight software for the Arizona State University SDSL (Sun Devil Satellite Lab) Coconut CubeSat. It is designed for FreeRTOS on a Raspberry Pi Pico (RP2040). This codebase is based on [this repo](https://github.com/smittytone/RP2040-FreeRTOS), so check it out for reference.

## Building and Deploying
Note that these instructions were only tested on `Ubuntu 20.04`
1. Navigate to the codebase in your terminal
   1. `cd ./coconut-fsw`
2. Install dependencies by running these two commands
   1. `sudo apt install -y python3 cmake gcc-arm-none-eabi build-essential git`
   2. `git submodule update --init --recursive` 
   3. Once you run these once, you never have to run them again
3. Build the firmware locally
   1. Run `./build.sh`
   2. You don't need any hardware to do this, just run this occasionally during development to see if your code is correct
4. Deploy the firmware to the board
   1. Hold the white `BOOTSEL` button on the RP2040
   2. Connect the RP2040 to your computer with Micro-USB
   3. Let go of the `BOOTSEL` button
   4. A USB flashdrive called `RPI-RP2` should connect to your computer
   5. Run `./deploy.sh`
   6. The codebase will now be running on the board. It will boot back into the flashed firmware even if powered off and powered on again 

## Debugging

For debugging, you have two options:
1. Printing to the debug log
   1. Run `ls /dev` and look for `ttyACM0` or `ttyACM1` 
   2. Run `debug/ACM0.sh` or `debug/ACM1.sh` to see the log 
   3. You will see any output from `logln_info` and `printf` calls in your code
2. Debugging through SWD/JTAG
   1. See `debug/README.md` to use the VSCode graphic debugger to set breakpoints and see variable values as your code executes

## Directory Structure
1. `main` Contains the main firmware for the satellite
2. `lib` Conatins external libraries (git submodules) used by the firmware
3. `freertos` Contains FreeRTOS configuration info
4. `scripts` Contains any testing/automation scripts unrelated to building
5. `.vscode` Contains VSCode extension configuration info and debugger setup
6. `docs` Contains markdown documents describing API functions and internal functions of certain subsystems
7. `build` Contains output from the build process (`build/Release/main/COCONUTFSW.uf2` is the main firmware output binary)