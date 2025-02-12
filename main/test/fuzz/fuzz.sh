#! /bin/bash

# RUN FROM ROOT PROJECT DIR
### Build
./deploy.sh -b -a
### Run Fuzzer
# export AFL_DEBUG=1
# export AFL_SKIP_CPUFREQ=1
export ASAN_OPTIONS=detect_leaks=0,abort_on_error=1,symbolize=0
export AFL_CUSTOM_MUTATOR_ONLY=1
export AFL_DISABLE_TRIM=1
export AFL_CUSTOM_MUTATOR_LIBRARY=$(pwd)/build/Fuzzer/main/test/fuzz/mutator/libcommand_mutator.so
export AFL_AUTORESUME=1
#afl-fuzz -i main/test/fuzz/corpus -o output -- ./build/Fuzzer/main/test/fuzz/command_fuzzer @@
aflr run --detached -n 8 -i main/test/fuzz/corpus -o output -t ./build/Fuzzer/main/test/fuzz/command_fuzzer -- @@
