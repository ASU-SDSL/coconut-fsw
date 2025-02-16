project ("coconut fuzzers")
cmake_minimum_required (VERSION 3.14)

set(project_name "fuzzer")

set(COMMAND_FUZZER_NAME "command_fuzzer")

# Protobuf
protobuf_generate_cpp(
            COMMAND_PROTO_SRC 
            COMMAND_PROTO_HDR
            command.proto)
            
set(COMMAND_PROTO_TARGET command_proto)
add_library(${COMMAND_PROTO_TARGET} STATIC ${COMMAND_PROTO_SRC} ${COMMAND_PROTO_HDR})

target_link_libraries(${COMMAND_PROTO_TARGET} PUBLIC Protobuf)

target_include_directories(${COMMAND_PROTO_TARGET} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(${COMMAND_PROTO_TARGET} PRIVATE "-fPIC")

if (CMAKE_CXX_COMPILER MATCHES "afl")
    target_compile_options(${COMMAND_PROTO_TARGET} PRIVATE
        --afl-noopt 
    )
endif()

# Fuzzer harness
add_executable(${COMMAND_FUZZER_NAME}
    command_fuzzer.cpp
    ${SOURCES}
)

target_compile_options(${COMMAND_FUZZER_NAME} PRIVATE 
    -fsanitize=address -fno-omit-frame-pointer 
)

target_link_libraries(${COMMAND_FUZZER_NAME} PRIVATE 
    -fsanitize=address
    Protobuf
    command_proto
    ${LINK_LIBS}
)

target_include_directories(${COMMAND_FUZZER_NAME}
    ${INTERNAL_INCLUDE_DIRS}
)

add_subdirectory(mutator)