# Coconut Flight Software

This repo contains the flight software for the SDSL Coconut CubeSat. It uses FreeRTOS on the Raspberry Pi Pico. This repo is based on [this repo](https://github.com/smittytone/RP2040-FreeRTOS) so check it out for reference.

## Building

1. Install Docker 🐋
2. Connect the board to your computer with a USB cable while holding the `BOOTSEL` button
   1. If you don't want to keep disconnecting and reconnecting the USB cable while testing code, the `RESET` button helps
3. Run `docker_deploy.sh` to build the firmware in Docker and deploy it to your board
   1. Run `docker_build.sh` if you'd just like to build your firmware without flashing it
   2. If you're getting this error when building on Windows: `env: bash\r: No such file or directory`, run `git config --global core.autocrlf false` then delete the repo, clone the repo again, then try building again
   3. If you're having issues flashing 

## Debugging

For debugging, you have two options:
1. Debugging with GDB through SWD/JTAG
   1. Instructions needed
2. Printing to the debug terminal
   1. Call log_info, log_debug, etc. inside your function
   2. Connect the board to your computer with a USB cable
   3. Install and open PUTTY with admin privileges 
   4. Select the `serial` option and use these options:
      1. Serial line
         1. `/dev/ttyACM0` on Linux
         2. `COM3` on Windows, may be different on your system so you should check the `Ports (COM & LPT)` tab in Device Manager
      2. Speed
         1. `115200`