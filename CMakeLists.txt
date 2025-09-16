cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 14)

# Set project data
set(BUILD_NUMBER "1")
set(PROJECT_NAME "COCONUTFSW")

# Read in password and hash it
set(ADMIN_TOKEN "COCONUT")
set(TOKEN_PATH ${CMAKE_CURRENT_LIST_DIR}/.token)
if (EXISTS ${TOKEN_PATH})
    file(READ ${TOKEN_PATH} ADMIN_TOKEN)
    message(STATUS "Found admin token: \"${ADMIN_TOKEN}\"")
else()
    # set default password if no password.txt exists
    message(STATUS "No admin token found in ${TOKEN_PATH}, using default token \"${ADMIN_TOKEN}\"")
endif ()
# save first 8 bytes of hash to use as password
execute_process(COMMAND bash "-c" "echo '${ADMIN_TOKEN}' | sha256sum | head -c 16 | sed 's/../\\\\x&/g'" OUTPUT_VARIABLE HASHED_TOKEN)
message(STATUS "SHA256 Hashed Token: \"${HASHED_TOKEN}\"")
add_definitions("-DADMIN_TOKEN=\"${HASHED_TOKEN}\"")

# Add build options
option(SIMULATOR "Enables simulator build" OFF)
option(FUZZER "Enables fuzzer build" OFF)
option(UNIT_TEST "Enables unit testing build" OFF)
set(DESKTOP_BUILD (SIMULATOR OR FUZZER OR UNIT_TEST)) # DESKTOP_BUILD means any builds not for embedded environment
IF (${DESKTOP_BUILD})
    # add preprocessor define so we can see this within C files
    message(STATUS "Added SIMULATOR define for ${PROJECT_NAME}")
    add_definitions(-DSIMULATOR) 
ENDIF()

#TinyAES
add_library(tinyaes STATIC 
    ${CMAKE_SOURCE_DIR}/lib/tiny-AES-c/aes.c
)

target_include_directories(tinyaes PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/tiny-AES-c
)

# Include the Pico SDK
IF (NOT ${DESKTOP_BUILD})
    include(pico_sdk_import.cmake) # - uses environment variable PICO_SDK_PATH
    pico_sdk_init() # Initialise the Pico SDK
ENDIF()

# Specify the app(s) source code
set(MAIN_DIRECTORY "${CMAKE_SOURCE_DIR}/main")

# FROM 1.3.0 -- Move common source code to a separate directory
set(COMMON_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/freertos/Common")

## Set flags and directory variables
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcommon")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcommon")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Name the project
project(${PROJECT_NAME}
    LANGUAGES C CXX ASM
    DESCRIPTION "Coconut Flight Software"
)

# Add FreeRTOS as a library
set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/freertos")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel")
set (FREERTOS_SOURCE_FILES
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c 
)

# Add FreeRTOS source files
IF(${DESKTOP_BUILD})
    # Simulator
    set(FREERTOS_SOURCE_FILES
        ${FREERTOS_SOURCE_FILES}
        ${CMAKE_SOURCE_DIR}/freertos/portable/ThirdParty/GCC/Posix/port.c
        ${CMAKE_SOURCE_DIR}/freertos/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    )
ELSE()
    # RP2040
    set(FREERTOS_SOURCE_FILES
        ${FREERTOS_SOURCE_FILES}
        ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0/port.c
    )
ENDIF()

add_library(FreeRTOS STATIC
    ${FREERTOS_SOURCE_FILES}
)

# Build FreeRTOS
set(FREERTOS_INCLUDE_FILES
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
)

# Setup FreeRTOS Includes
IF (${DESKTOP_BUILD})
    # Simulator
    set(FREERTOS_INCLUDE_FILES
        ${FREERTOS_INCLUDE_FILES}
        ${CMAKE_SOURCE_DIR}/freertos/portable/ThirdParty/GCC/Posix
        ${CMAKE_SOURCE_DIR}/freertos/portable/ThirdParty/GCC/Posix/utils
    )
ELSE()
    # RP2040
    set(FREERTOS_INCLUDE_FILES
        ${FREERTOS_INCLUDE_FILES}
        ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
    )
ENDIF()

target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_INCLUDE_FILES}
)

# Include AFLplusplus_ultimate_protobuf_mutator for fuzzing builds
IF (FUZZER)
    include(${CMAKE_SOURCE_DIR}/lib/AFLplusplus_ultimate_protobuf_mutator/cmake/libprotobuf.cmake)
    include(${CMAKE_SOURCE_DIR}/lib/AFLplusplus_ultimate_protobuf_mutator/cmake/libprotobuf-mutator.cmake)
    set(LIB_PROTOBUF_DOWNLOAD CACHE INTERNAL OFF)
    set(LIB_PROTOBUF_MUTATOR_DOWNLOAD CACHE INTERNAL OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/AFLplusplus_ultimate_protobuf_mutator)
ENDIF()

# Include libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/fatfs/ build)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/libspacepacket/)
IF (NOT ${DESKTOP_BUILD})
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/RadioLib)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/onewire_library/)
ENDIF()

# Include the app's source code
add_subdirectory(${MAIN_DIRECTORY})
