Steps:

1. Build OpenOCD and "make install" it
   1. Tested with this commit: https://github.com/openocd-org/openocd/commit/9ea7f3d647c8ecf6b0f1424002dfc3f4504a162c
2. Put "set USE_CORE 0" inside "/usr/local/share/openocd/scripts/target/rp2040.cfg" or just replace it with the rp2040.cfg in this folder
3. Connect the PicoProbe to the board you want to debug
   1. This document has pinout and is very useful in general: https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf 
4. Install "Cortex Debug" extension in VSCode
5. Run debug.sh and connect using gdb-multiarch OR use the VSCode debugging profile
   1. This doc can be useful: https://mcuoneclipse.com/2023/10/22/debug-probes-for-rp2040-with-vs-code/
   2. Make sure to install gdb-multiarch with "sudo apt install gdb-multiarch" as well