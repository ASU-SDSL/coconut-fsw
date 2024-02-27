# https://mcuoneclipse.com/2023/10/22/debug-probes-for-rp2040-with-vs-code/
# https://github.com/raspberrypi/debugprobe/releases/tag/picoprobe-cmsis-v1.1
# https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf
# https://pfeerick.github.io/InfiniTime/doc/openOCD.html
# https://github.com/openocd-org/openocd/commit/9ea7f3d647c8ecf6b0f1424002dfc3f4504a162c

sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "set USE_CORE 0" -s tcl