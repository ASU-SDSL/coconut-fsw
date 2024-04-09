# https://machinehum.medium.com/flashing-the-rp2040-with-a-jlink-and-openocd-b5c6806d51c2
# Must be run from root project dir (like .vscode/tasks.json is doing)

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