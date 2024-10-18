project ("coconut fuzzers")
cmake_minimum_required (VERSION 3.14)

set(project_name "fuzzer")

set(COMMAND_FUZZER_NAME "command_fuzzer")

# Fuzzer binary
add_executable(${COMMAND_FUZZER_NAME}
    command_fuzzer.cpp

    ${MAIN_DIRECTORY}/tasks/gse/gse.c
    ${MAIN_DIRECTORY}/tasks/steve/steve.c
    ${MAIN_DIRECTORY}/tasks/command/command.c
    ${MAIN_DIRECTORY}/tasks/telemetry/telemetry.c
    ${MAIN_DIRECTORY}/tasks/filesystem/filesystem.c

    ${MAIN_DIRECTORY}/tasks/steve/jobs/heartbeat_job.c

    ${MAIN_DIRECTORY}/utilities/src/ccsds.c
    ${MAIN_DIRECTORY}/utilities/src/log.c
    ${MAIN_DIRECTORY}/utilities/src/timing.c

    ${MAIN_DIRECTORY}/drivers/src/i2c.c
    ${MAIN_DIRECTORY}/drivers/src/eps.c
    ${MAIN_DIRECTORY}/drivers/src/rtc.c
    ${MAIN_DIRECTORY}/drivers/src/mag.c
    ${MAIN_DIRECTORY}/drivers/src/usb_uart.c

    ${MAIN_DIRECTORY}/drivers/src/vega_ant.c

    ${MAIN_DIRECTORY}/simulator/drivers/src/diskio.c
)

target_compile_options(${COMMAND_FUZZER_NAME}
    PRIVATE -g -O1 -fsanitize=address -fno-omit-frame-pointer 
)

target_link_libraries(${COMMAND_FUZZER_NAME}
    PRIVATE -fsanitize=address
    ${LINK_LIBS}
)

target_include_directories(${COMMAND_FUZZER_NAME}
    ${INTERNAL_INCLUDE_DIRS}
)
