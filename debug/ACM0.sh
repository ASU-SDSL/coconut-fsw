# ctrl a then k to exit
# if you get this error immediately (run this then logout and log back in): sudo usermod -a -G dialout $USER (
# if you get this error after unplugging always: [screen is terminating], type this: killall screen
screen /dev/ttyACM0 115200