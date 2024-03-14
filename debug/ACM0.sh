# ctrl a ctrl d to exit
# if you get this error: [screen is terminating], type this: killall SCREEN
gnome-terminal -- bash -c "screen /dev/ttyACM0 115200; exec bash"