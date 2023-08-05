cmake_minimum_required(VERSION 3.14)

# NOTE Equivalent of `#define DEBUG 1`
if(${DO_DEBUG})
    add_compile_definitions(DEBUG=1)
    message(STATUS "Debugging enabled for ${PROJECT_NAME}")
else()
    message(STATUS "Debugging disabled for ${PROJECT_NAME}")
endif()

# Make project data accessible to compiler
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    ${MAIN_DIRECTORY}/src/main.c
    ${MAIN_DIRECTORY}/src/gse.c
    ${MAIN_DIRECTORY}/src/steve.c
    ${MAIN_DIRECTORY}/src/radio.cpp
    ${MAIN_DIRECTORY}/src/command.c
    ${MAIN_DIRECTORY}/src/EpsSensor.c
    ${MAIN_DIRECTORY}/src/telemetry.c
    ${MAIN_DIRECTORY}/src/log.c
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

#include_directories(${MAIN_DIRECTORY}../lib/RadioLib)
#include_directories(${MAIN_DIRECTORY}../lib/spacepacket/includes)
#link_directories(${MAIN_DIRECTORY}../lib/spacepacket/includes)

# Link to built libraries
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    pico_stdlib
    FreeRTOS
    hardware_spi
    hardware_adc
    hardware_i2c
    RadioLib
    #spacepacket - note: for now, just include spacepacket as an executable if its needed, otherwise we will probably have to make it a library through its
    #               cmakelists file
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${MAIN_DIRECTORY}/include
)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${PROJECT_NAME})
