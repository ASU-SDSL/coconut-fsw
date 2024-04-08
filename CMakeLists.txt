cmake_minimum_required(VERSION 3.14)

# Set project data
set(BUILD_NUMBER "1")

set(PROJECT_NAME "COCONUTFSW")

# Include the Pico SDK
include(pico_sdk_import.cmake) # - uses environment variable PICO_SDK_PATH
pico_sdk_init() # Initialise the Pico SDK

# Specify the app(s) source code
set(MAIN_DIRECTORY "${CMAKE_SOURCE_DIR}/main")

# FROM 1.3.0 -- Move common source code to a separate directory
set(COMMON_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/freertos/Common")

# Set app-side debugging "ON" or "OFF"
set(DO_DEBUG "ON")

# Set flags and directory variables

# Simulator
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcommon")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcommon")

# RP2040
# set(CMAKE_C_FLAGS "-fcommon -march=armv6-m -mcpu=cortex-m0plus")
# set(CMAKE_CXX_FLAGS "-fcommon -march=armv6-m -mcpu=cortex-m0plus")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DTSDEBUG -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTSDEBUG -g -O0")
set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/freertos")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel")

# Name the project
project(${PROJECT_NAME}
    LANGUAGES C CXX ASM
    DESCRIPTION "Coconut Flight Software"
)

# Add FreeRTOS as a library
set (FREERTOS_SOURCE_FILES
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c 
)

# Simulator
set(FREERTOS_SOURCE_FILES
    ${FREERTOS_SOURCE_FILES}
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/Posix/port.c
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

# RP2040
# set(FREERTOS_SOURCE_FILES
#     ${FREERTOS_SOURCE_FILES}
#     ${FREERTOS_SRC_DIRECTORY}/portable/ThirdPartyGCC/ARM_CM0/port.c
# )

add_library(FreeRTOS STATIC
    ${FREERTOS_SOURCE_FILES}
)


# Build FreeRTOS
set(FREERTOS_INCLUDE_FILES
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
)

# Simulator
set(FREERTOS_INCLUDE_FILES
    ${FREERTOS_INCLUDE_FILES}
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/Posix
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/Posix/utils
)

# RP2040
# set(FREERTOS_INCLUDE_FILES
#     ${FREERTOS_INCLUDE_FILES}
#     ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
# )

target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_INCLUDE_FILES}
)

# Include the app's source code
add_subdirectory(${MAIN_DIRECTORY})

# add libraries here (and in target_link_libraries under the main cmakelists)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/RadioLib)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
