project ("filesystem unit test")
cmake_minimum_required (VERSION 3.13)

set(project_name "filesystem_test")

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