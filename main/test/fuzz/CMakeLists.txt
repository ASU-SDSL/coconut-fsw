project ("coconut fuzzers")
cmake_minimum_required (VERSION 3.14)

# ====================  Define your project name (edit) ========================
set(project_name "fuzzer")

set(COMMAND_FUZZER_NAME "command_fuzzer")
set(TEST_PROTO_CREATOR_NAME "test_proto_creator")

find_package(Protobuf REQUIRED)

# Import mutator
add_subdirectory(./mutator command_proto_mutator)

# Fuzzer binary
add_executable(${COMMAND_FUZZER_NAME}
    ${TEST_PROTO_SRC}
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

add_dependencies(${COMMAND_FUZZER_NAME} command_proto_mutator)

target_compile_options(${COMMAND_FUZZER_NAME}
    PRIVATE -g -O1 -fno-omit-frame-pointer 
)

target_link_libraries(${COMMAND_FUZZER_NAME}
    # PRIVATE -fsanitize=address
    protobuf
    ${LINK_LIBS}
)

target_include_directories(${COMMAND_FUZZER_NAME}
    ${INTERNAL_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS} 
)

# Testing binary
add_executable(${TEST_PROTO_CREATOR_NAME}
    ${TEST_PROTO_SRC} 
    test_proto_creator.cpp
)

# target_compile_options(${TEST_PROTO_CREATOR_NAME}
#     PRIVATE --afl-noopt # disable afl instrumentation for test binary
# )

add_dependencies(${TEST_PROTO_CREATOR_NAME} command_proto_mutator)

target_link_libraries(${TEST_PROTO_CREATOR_NAME}
    protobuf
    command_proto_mutator
)

target_include_directories(${TEST_PROTO_CREATOR_NAME}
    ${INTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)