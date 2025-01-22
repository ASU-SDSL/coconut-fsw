cmake_minimum_required(VERSION 3.14)

## Add debug preprocessor flag if building for Debug target
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
    message(STATUS "Debugging enabled for ${PROJECT_NAME}")
else()
    message(STATUS "Debugging disabled for ${PROJECT_NAME}")
endif()

## Make project data accessible to compiler
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

## Create a sources variable with a link to all cpp files to compile
set(SOURCES
    ${MAIN_DIRECTORY}/tasks/main/main.c
    ${MAIN_DIRECTORY}/tasks/gse/gse.c
    ${MAIN_DIRECTORY}/tasks/steve/steve.c
    ${MAIN_DIRECTORY}/tasks/command/command.c
    ${MAIN_DIRECTORY}/tasks/telemetry/telemetry.c
    ${MAIN_DIRECTORY}/tasks/filesystem/filesystem.c

    ${MAIN_DIRECTORY}/tasks/steve/jobs/heartbeat_job.c

    ${MAIN_DIRECTORY}/utilities/src/ccsds.c
    ${MAIN_DIRECTORY}/utilities/src/log.c
    ${MAIN_DIRECTORY}/utilities/src/timing.c
    ${MAIN_DIRECTORY}/utilities/src/user_auth.c

    ${MAIN_DIRECTORY}/drivers/src/i2c.c
    ${MAIN_DIRECTORY}/drivers/src/eps.c
    ${MAIN_DIRECTORY}/drivers/src/rtc.c
    ${MAIN_DIRECTORY}/drivers/src/mag.c
    ${MAIN_DIRECTORY}/drivers/src/vega_ant.c
)

# build certain files depending on simulator build
IF (SIMULATOR OR FUZZER) 
    set (SOURCES
        ${SOURCES}
        ${MAIN_DIRECTORY}/simulator/drivers/src/diskio.c
        ${MAIN_DIRECTORY}/simulator/drivers/src/usb_uart.c
    )
ELSE()
    set (SOURCES
        ${SOURCES}
        ${MAIN_DIRECTORY}/drivers/src/diskio.c
        ${MAIN_DIRECTORY}/tasks/radio/radio.cpp
        ${MAIN_DIRECTORY}/drivers/src/mram.c
        ${MAIN_DIRECTORY}/drivers/src/usb_uart.c
    )
ENDIF()

add_executable(${PROJECT_NAME}
    ${SOURCES}
)


## Link to built libraries
set (LINK_LIBS
    FreeRTOS
    FatFs
)

IF (SIMULATOR OR FUZZER)
    # add pthread for freertos unix port
    set (LINK_LIBS
        ${LINK_LIBS}
        pthread
    )
ELSE()
    # add picolib only if not building for simulator
    #   this is because it only builds for ARM arch's
    #   other libs are just currently not supported
    set (LINK_LIBS
        ${LINK_LIBS}
        pico_stdlib
        hardware_spi
        hardware_adc
        hardware_i2c
        RadioLib
    )
ENDIF()

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    ${LINK_LIBS}
)

## Setup internal include dirs
set (INTERNAL_INCLUDE_DIRS
    PRIVATE ${MAIN_DIRECTORY}/tasks/main
    PRIVATE ${MAIN_DIRECTORY}/tasks/command
    PRIVATE ${MAIN_DIRECTORY}/tasks/gse
    PRIVATE ${MAIN_DIRECTORY}/tasks/steve
    PRIVATE ${MAIN_DIRECTORY}/tasks/steve/jobs
    PRIVATE ${MAIN_DIRECTORY}/tasks/telemetry
    PRIVATE ${MAIN_DIRECTORY}/tasks/filesystem

    PRIVATE ${MAIN_DIRECTORY}/utilities/include
    PRIVATE ${MAIN_DIRECTORY}/drivers/include
)


IF (SIMULATOR OR FUZZER)
    # enable stubs for simulator
    set(INTERNAL_INCLUDE_DIRS
        ${INTERNAL_INCLUDE_DIRS}
        PRIVATE ${MAIN_DIRECTORY}/simulator/pico-sdk
        PRIVATE ${MAIN_DIRECTORY}/simulator/tasks/radio
    )
ELSE() 
    # enable radio module if not simulator
    set(INTERNAL_INCLUDE_DIRS
        ${INTERNAL_INCLUDE_DIRS}
        PRIVATE ${MAIN_DIRECTORY}/tasks/radio
    )
ENDIF()

target_include_directories(${PROJECT_NAME} 
    ${INTERNAL_INCLUDE_DIRS}
)

# Enable/disable STDIO via USB/UART
#   Similar to above situation, only use picolib if building for flight hardwares
IF (NOT SIMULATOR AND NOT FUZZER)
    include_directories(${MAIN_DIRECTORY}../deps/RadioLib)
    pico_enable_stdio_uart(${PROJECT_NAME} 0)
    pico_enable_stdio_usb(${PROJECT_NAME} 1)
    
    pico_add_extra_outputs(${PROJECT_NAME})
ENDIF()

# Build fuzzers
IF (FUZZER)
    add_subdirectory(${MAIN_DIRECTORY}/test/fuzz)
ENDIF()

# Build unit tests
IF (SIMULATOR)
    add_subdirectory(${MAIN_DIRECTORY}/test/unit)
ENDIF()