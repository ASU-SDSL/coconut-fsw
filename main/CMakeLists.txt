cmake_minimum_required(VERSION 3.14)

# Should we set app-level debugging?
# NOTE Equivalent of `#define DEBUG 1`
if(${DO_DEBUG})
    add_compile_definitions(DEBUG=1)
    message(STATUS "App-side debugging enabled for ${PROJECT_NAME}")
else()
    message(STATUS "App-side debugging disabled for ${PROJECT_NAME}")
endif()

# Make project data accessible to compiler
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    ${FSW_DIRECTORY}/src/main.c
    ${FSW_DIRECTORY}/src/gse.c
    ${FSW_DIRECTORY}/src/missionmanager.c
    ${FSW_DIRECTORY}/src/radio.c
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Link to built libraries
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    pico_stdlib
    FreeRTOS)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${FSW_DIRECTORY}/include
)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${PROJECT_NAME})
