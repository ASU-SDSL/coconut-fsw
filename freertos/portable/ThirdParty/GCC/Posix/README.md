A modified FreeRTOS POSIX port to use SIGUSR1 instead of SIGALRM for tick simulation
This allows for the firmware to run inside of LibFuzzer with the flag `-handle_usr1=0`