set(COMMAND_MUTATOR_TARGET command_mutator)
add_library(${COMMAND_MUTATOR_TARGET} SHARED
    mutator.cpp
)

target_link_libraries(${COMMAND_MUTATOR_TARGET} PRIVATE 
    afl_ultimate_protobuf_mutator
    command_proto
)

target_include_directories(${COMMAND_MUTATOR_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_CXX_COMPILER MATCHES "afl")
    # disable AFL instrumentation for mutator if AFL compiler is used, otherwise it will throw an error when loaded
    target_compile_options(${COMMAND_MUTATOR_TARGET} PRIVATE 
        --afl-noopt 
    )
endif()