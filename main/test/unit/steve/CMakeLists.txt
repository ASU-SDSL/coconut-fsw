project ("steve unit test")
cmake_minimum_required (VERSION 3.13)

set(project_name "steve_test")

# =====================  Create your mock here  ========================

# set(mock_name "${project_name}_mock")

# list(APPEND mock_list
    # ${MAIN_DIRECTORY}/tasks/steve/steve.h
# )
        
# list(APPEND mock_include_list
    # ${FREERTOS_CFG_DIRECTORY}/
    # ${FREERTOS_SRC_DIRECTORY}/include
    # ${CMAKE_SOURCE_DIR}/freertos/portable/ThirdParty/GCC/Posix
    # ${CMAKE_SOURCE_DIR}/freertos/portable/ThirdParty/GCC/Posix/utils

    # ${MAIN_DIRECTORY}/tasks/steve
    # ${MAIN_DIRECTORY}/tasks/steve/jobs
    # ${MAIN_DIRECTORY}/tasks/telemetry
    # ${MAIN_DIRECTORY}/tasks/gse
    # ${MAIN_DIRECTORY}/tasks/command
    # ${MAIN_DIRECTORY}/utilities/include
    # ${MAIN_DIRECTORY}/drivers/include

    # ${MAIN_DIRECTORY}/simulator/pico-sdk
    # ${MAIN_DIRECTORY}/simulator/tasks/radio
# )

# list(APPEND mock_define_list
#     portHAS_STACK_OVERFLOW_CHECKING=1
# )


# create_mock_list(${mock_name}
#     "${mock_list}"
#     ${CMAKE_SOURCE_DIR}/main/test/unit/project.yml
#     "${mock_include_list}"
#     "${mock_define_list}"
# )

# Code/library under test

list(APPEND real_source_files
    ${SOURCES}
)

list(APPEND real_include_directories
    ${FREERTOS_INCLUDE_FILES}
    ${INTERNAL_INCLUDE_DIRS}
)
# Unit test binary

list(APPEND test_include_directories
    ${FREERTOS_INCLUDE_FILES}
    ${INTERNAL_INCLUDE_DIRS}
)

set(real_name "${project_name}_real")

create_real_library(${real_name}
    "${real_source_files}"
    "${real_include_directories}"
    "${mock_name}"
)

target_link_libraries(${real_name} LINK_PUBLIC
    ${LINK_LIBS}
)

list(APPEND utest_link_list
    # -l${mock_name}
    ${real_name}
    -lpthread
    ${LINK_LIBS}
)

set(utest_name ${project_name}_utest)
set(utest_source ${project_name}_unittest.c)
create_test(${utest_name}
    ${utest_source}
    "${utest_link_list}"
    "${utest_dep_list}"
    "${test_include_directories}"
)