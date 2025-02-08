project ("steve unit test")
cmake_minimum_required (VERSION 3.13)

# ====================  Define your project name (edit) ========================
set(project_name "steve_test")

# =====================  Create your mock here  (edit)  ========================

# list the files to mock here
list(APPEND mock_list
    # ${MAIN_DIRECTORY}/tasks/steve/steve.h
)
        
# list the directories your mocks need
list(APPEND mock_include_list
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
)
#list the definitions of your mocks to control what to be included
list(APPEND mock_define_list
    portHAS_STACK_OVERFLOW_CHECKING=1
)
# ================= Create the library under test here (edit) ==================

# list the files you would like to test here
list(APPEND real_source_files
    ${MAIN_DIRECTORY}/tasks/main/main.c
    ${MAIN_DIRECTORY}/tasks/gse/gse.c
    ${MAIN_DIRECTORY}/tasks/steve/steve.c
    ${MAIN_DIRECTORY}/tasks/command/command.c
    ${MAIN_DIRECTORY}/tasks/telemetry/telemetry.c
    ${MAIN_DIRECTORY}/tasks/filesystem/filesystem.c

    ${MAIN_DIRECTORY}/tasks/steve/jobs/heartbeat_job.c

    ${MAIN_DIRECTORY}/utilities/src/timing.c

    ${MAIN_DIRECTORY}/drivers/src/i2c.c
    ${MAIN_DIRECTORY}/drivers/src/eps.c
    ${MAIN_DIRECTORY}/drivers/src/rtc.c
    ${MAIN_DIRECTORY}/drivers/src/mag.c
    ${MAIN_DIRECTORY}/drivers/src/vega_ant.c

    ${MAIN_DIRECTORY}/simulator/drivers/src/diskio.c
    ${MAIN_DIRECTORY}/simulator/drivers/src/usb_uart.c

    ${MAIN_DIRECTORY}/test/unit/utilities/log.c
)

# list the directories the module under test includes
list(APPEND real_include_directories
    ${FREERTOS_INCLUDE_FILES}
    ${INTERNAL_INCLUDE_DIRS}
)
# =====================  Create UnitTest Code here (edit)  =====================

# list the directories your test needs to include
list(APPEND test_include_directories
    ${FREERTOS_INCLUDE_FILES}
    ${INTERNAL_INCLUDE_DIRS}
)

# =============================  (end edit)  ===================================

# set(mock_name "${project_name}_mock")
set(real_name "${project_name}_real")

# create_mock_list(${mock_name}
#     "${mock_list}"
#     ${CMAKE_SOURCE_DIR}/main/test/unit/project.yml
#     "${mock_include_list}"
#     "${mock_define_list}"
# )

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
    lib${real_name}.a
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