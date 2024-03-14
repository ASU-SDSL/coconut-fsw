# ctrl a ctrl d to exit
# if you get this error: [screen is terminating], type this: killall screen
gnome-terminal -- bash -c "screen /dev/ttyUSB0 115200; exec bash"