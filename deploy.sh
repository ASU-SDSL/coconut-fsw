#!/usr/bin/env bash

############# YOU CAN CHANGE THESE ##################

rpi_path="/media/${USER}/RPI-RP2"       # The path of the RPI's file system when mounted

#####################################################

# GLOBALS
timeout=30
do_build=1
do_flash=1
uf2_path="UNDEFINED"
cmake_path="$PWD/CMakeLists.txt"

set -e

# FUNCTIONS
show_help() {
    echo -e "Usage:\n"
    echo -e "   deploy [-b] [-u]"
    echo -e "Options:\n"
    echo "  -b / --build    ONLY build the project."
    echo "  -f / --flash    ONLY flash the project"
    echo "  -h / --help     Show this help screen"
    echo
}

# I don't think that this is really needed
<<comment  
update_build_number() {
    build_val=$(grep 'set(BUILD_NUMBER "' ${cmake_path})
    old_num=$(echo "${build_val}" | cut -d '"' -s -f 2)
    ((new_num=old_num+1))

    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        sed -i "s|BUILD_NUMBER \"${old_num}\"|BUILD_NUMBER \"${new_num}\"|" "${cmake_path}"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS requires slightly different syntax from Unix
        sed -i '' "s|BUILD_NUMBER \"${old_num}\"|BUILD_NUMBER \"${new_num}\"|" "${cmake_path}"
    else
        echo "[ERROR] Unknown OS... build number not incremented"
    fi
}
comment

uf2_path="build/main/COCONUTFSW.uf2"

# RUNTIME START
for arg in "$@"; do
    check_arg=${arg,,}
    echo "check: $check_arg"
    if [[ "$check_arg" = "--help" || "$check_arg" = "-h" ]]; then
        show_help
        exit 0
    elif [[ "$check_arg" = "--build" || "$check_arg" = "-b" ]]; then # only build
        do_flash=0
    elif [[ "$check_arg" = "-flash" || "$check_arg" = "-f" ]]; then # only flash
        do_build=0 
    fi
done

# Do we build?
err=0
if [[ ${do_build} -eq 1 ]]; then
    # FROM 1.1.0 -- auto-update the build number
    #update_build_number
    
    if [[ ! -e "./build" ]]; then
        # No build folder? Then create it
        # and configure the build
        cmake -S . -B build/ -D "CMAKE_C_COMPILER:FILEPATH=$(which arm-none-eabi-gcc)" -D CMAKE_BUILD_TYPE:STRING=Release
        err=$?
    fi
    
    # Build the app
    cmake --build build
    err=$?
fi

# Check for errors
if [[ ${err} -ne 0 ]]; then
    exit 1
fi

# Do we flash?
if [ ${do_flash} -eq 1 ]; then
    # Wait for the RPI_R2 mount
    count=0
    if [ ! -d "${rpi_path}" ]; then
        echo "Waiting for RP2040 device to mount"
        while [ ! -d "${rpi_path}" ]; do
            sleep 1
            ((count+=1))
            if [[ $count -eq $timeout ]]; then
                echo "[ERROR] RP2040 device not mounted after ${timeout}s... exiting"
                exit 1
            fi
        done
    fi

    echo "RP2040 device mounted..."

    # Check for available app file
    if [ ! -f "${uf2_path}" ]; then
        echo "[ERROR] Cannot find file ${uf2_path}... exiting"
        exit 1
    fi

    echo "Copying ${uf2_path} to ${rpi_path}/${uf2_path##*/}"

    # Copy file
    if cp -f "${uf2_path}" "${rpi_path}/${uf2_path##*/}"; then
        echo "${uf2_path##*/} copied to ${rpi_path}"
    else
        echo "[ERROR] Could not copy ${uf2_path##*/} to ${rpi_path}/${uf2_path##*/}"
        exit 1
    fi
fi

exit 0
