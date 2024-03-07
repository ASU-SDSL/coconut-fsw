killall screen
gnome-terminal -- bash -c "screen /dev/ttyUSB0 115200; exec bash"
gnome-terminal -- bash -c "screen /dev/ttyACM0 115200; exec bash"
gnome-terminal -- bash -c "screen /dev/ttyACM1 115200; exec bash"