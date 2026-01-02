#!/bin/bash
# flashes fsw in Release mode (no debug) 

./deploy.sh -b

echo "Flashing target device..."

sudo openocd -f interface/cmsis-dap.cfg -f debug/rp2040.cfg -c "adapter speed 5000" -c "program build/Release/main/COCONUTFSW.elf verify reset exit"

echo "Done."