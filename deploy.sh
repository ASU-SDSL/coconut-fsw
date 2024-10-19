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

# RUNTIME START
debug_build=0
simulator_build=0
for arg in "$@"; do
    check_arg=${arg,,}
    echo "check: $check_arg"
    if [[ "$check_arg" = "--help" || "$check_arg" = "-h" ]]; then
        show_help
        exit 0
    elif [[ "$check_arg" = "--build" || "$check_arg" = "-b" ]]; then # only build
        do_flash=0
    elif [[ "$check_arg" = "--flash" || "$check_arg" = "-f" ]]; then # only flash
        do_build=0 
    elif [[ "$check_arg" = "--debug" || "$check_arg" = "-g" ]]; then # enable debug build
        debug_build=1
    elif [[ "$check_arg" = "--sim" || "$check_arg" = "-s" ]]; then # enable debug build
        simulator_build=1
    elif [[ "$check_arg" = "--afl" || "$check_arg" = "-a" ]]; then # enable fuzzer build
        fuzzer_build=1
        simulator_build=1
    fi
done

# Do we build?
err=0
if [[ ${do_build} -eq 1 ]]; then
    # FROM 1.1.0 -- auto-update the build number
    #update_build_number

    # resolve build string
    build_string="Release"
    if [[ ${debug_build} -eq 1 ]]; then
        build_string="Debug"
    elif [[ ${fuzzer_build} -eq 1 ]]; then
        build_string="Fuzzer"
    elif [[ ${simulator_build} -eq 1 ]]; then
        build_string="Simulator"
    fi
    echo $build_string
    
    # generate cmake files
    build_path=build/${build_string}
    uf2_path="${build_path}/main/COCONUTFSW.uf2"

    if [[ ${fuzzer_build} -eq 1 ]]; then
	# use Release here because otherwise you have to include protobufd instead of protobuf
        export AFL_LLVM_THREADSAFE_INST=1
        cmake -S . -B ${build_path} \
            -D "CMAKE_C_COMPILER:FILEPATH=$(which afl-clang-fast)" \
            -D "CMAKE_CXX_COMPILER:FILEPATH=$(which afl-clang-fast++)" \
            -D CMAKE_BUILD_TYPE:STRING="Release" -DFUZZER:BOOL=ON
    elif [[ ${simulator_build} -eq 1 ]]; then
        cmake -S . -B ${build_path} \
            -D "CMAKE_C_COMPILER:FILEPATH=$(which gcc)" \
            -D "CMAKE_CXX_COMPILER:FILEPATH=$(which g++)" \
            -D CMAKE_BUILD_TYPE:STRING="Debug" -DSIMULATOR:BOOL=ON 
    else
        cmake -S . -B ${build_path} \
            -D "CMAKE_C_COMPILER:FILEPATH=$(which arm-none-eabi-gcc)" \
            -D "CMAKE_CXX_COMPILER:FILEPATH=$(which arm-none-eabi-g++)" \
            -D CMAKE_BUILD_TYPE:STRING="${build_string}"
    fi
    
    err=$?
    
    # build
    cmake --build ${build_path} -j $(nproc)
    err=$?
fi

# Check for errors
if [[ ${err} -ne 0 ]]; then
    exit 1
fi

# Do we flash?
if [[ ${do_flash} -eq 1 && ${simulator_build} -ne 1 ]]; then
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
