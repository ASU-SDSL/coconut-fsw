A modified FreeRTOS POSIX port to use SIGUSR1 instead of SIGALRM for tick simulation
This allows for the firmware to run inside of LibFuzzer with the flag `-handle_usr2=0`
Also allows for execution in AFL++ as it only catches SIGUSR1 to allow for skipping inputs