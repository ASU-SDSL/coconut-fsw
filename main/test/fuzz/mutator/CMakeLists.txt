project ("coconut proto mutator")
cmake_minimum_required (VERSION 3.14)

# ====================  Define your project name (edit) ========================
set(project_name "command_proto_mutator")

set(MUTATOR_NAME "command_proto_mutator")

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# Import protobuf
find_package(Protobuf REQUIRED)

# Mutator lib
protobuf_generate_cpp(
            TEST_PROTO_SRC 
            TEST_PROTO_HDR
            command.proto)

add_library(${MUTATOR_NAME} SHARED
    ${TEST_PROTO_SRC} 
    command_proto_mutator.cpp
)

target_compile_options(${MUTATOR_NAME}
    PRIVATE -g -O1 -fno-omit-frame-pointer # disable afl instrumentation for protobuf lib
)

add_dependencies(${MUTATOR_NAME} aflpm)

target_link_libraries(${MUTATOR_NAME}
    aflpm
)

target_include_directories(${MUTATOR_NAME}
    ${INTERNAL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/lib/AFLplusplus-protobuf-mutator/src
    ${CMAKE_CURRENT_BINARY_DIR}
)
