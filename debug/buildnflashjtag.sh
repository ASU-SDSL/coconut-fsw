# Build debug binaries
./deploy.sh -b -g

# Check if we build successfully
if [ $? != 0 ]; then
    echo >&2 "\033[0;31m \nFailed to build!"
    exit 1
else
    exit 0
fi


# Flash
openocd -f interface/cmsis-dap.cfg -f ./debug/rp2040.cfg -c "adapter speed 5000" -s tcl -c "program ./build/main/COCONUTFSW.elf reset exit"