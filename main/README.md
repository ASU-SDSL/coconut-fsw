## main

The `main` directory contains most of the firmware used on the satellite and is organized in the following way:

1. The `tasks` directory contains all of the files that relate to running tasks in the FreeRTOS (each `.c` file in `src` represents a FreeRTOS task)
2. The `drivers` directory contains all of the code relating to interacting with devices with the RPi Pico. These typically have many pico-sdk calls.
3. The `utilities` directory conatin any other code that is not directly related to a task or device driver.