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
    ${MAIN_DIRECTORY}/tasks/radio/radio.cpp
    ${MAIN_DIRECTORY}/tasks/command/command.c
    ${MAIN_DIRECTORY}/tasks/telemetry/telemetry.c

    ${MAIN_DIRECTORY}/tasks/steve/jobs/heartbeat_job.c

    ${MAIN_DIRECTORY}/utilities/src/ccsds.c
    ${MAIN_DIRECTORY}/utilities/src/log.c
    ${MAIN_DIRECTORY}/utilities/src/timing.c

    ${MAIN_DIRECTORY}/drivers/src/i2c.c
    ${MAIN_DIRECTORY}/drivers/src/eps.c
    ${MAIN_DIRECTORY}/drivers/src/rtc.c
    ${MAIN_DIRECTORY}/drivers/src/mag.c
    ${MAIN_DIRECTORY}/drivers/src/hw_config.c
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

#include_directories(${MAIN_DIRECTORY}../deps/RadioLib)
#include_directories(${MAIN_DIRECTORY}../deps/spacepacket/includes)
#link_directories(${MAIN_DIRECTORY}../deps/spacepacket/includes)

## Link to built libraries
set (LINK_LIBS
    FreeRTOS
    hardware_spi
    hardware_adc
    hardware_i2c
    RadioLib
    FatFs_SPI
    pthread
)

# add picolib only if not building for simulator
#   this is because it only builds for ARM arch's
IF (NOT SIMULATOR)
    set (LINK_LIBS
        ${LINK_LIBS}
        pico_stdlib
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
    PRIVATE ${MAIN_DIRECTORY}/tasks/radio
    PRIVATE ${MAIN_DIRECTORY}/tasks/steve
    PRIVATE ${MAIN_DIRECTORY}/tasks/steve/jobs
    PRIVATE ${MAIN_DIRECTORY}/tasks/telemetry

    PRIVATE ${MAIN_DIRECTORY}/utilities/include
    PRIVATE ${MAIN_DIRECTORY}/drivers/include
)


IF (SIMULATOR)
    set(INTERNAL_INCLUDE_DIRS
        ${INTERNAL_INCLUDE_DIRS}
        PRIVATE ${MAIN_DIRECTORY}/simulator
    )
ENDIF()

target_include_directories(${PROJECT_NAME} 
    ${INTERNAL_INCLUDE_DIRS}
)

# Enable/disable STDIO via USB/UART
#   Similar to above situation, only use picolib if building for flight hardwares
IF (NOT SIMULATOR)
    pico_enable_stdio_uart(${PROJECT_NAME} 0)
    pico_enable_stdio_usb(${PROJECT_NAME} 1)
    
    pico_add_extra_outputs(${PROJECT_NAME})
ENDIF()

