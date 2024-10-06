# rm -rf build
./deploy.sh -b -a
# export AFL_DEBUG=1
export AFL_SKIP_CPUFREQ=1
# export AFL_CUSTOM_MUTATOR_LIBRARY=${HOME}/Documents/Github/AFLplusplus/custom_mutators/libfuzzer/libfuzzer-mutator.so
export ASAN_OPTIONS=detect_leaks=0,abort_on_error=1,symbolize=0
# ./build/Simulator/main/command_fuzzer -jobs=4 -artifact_prefix=crashes -close_fd_mask=1 -ignore_crashes=1 -print_pcs=1 -timeout=5 -handle_usr1=0 -detect_leaks=0 -fork=1
afl-fuzz -t 1000 -i input -o output -- ./build/Fuzzer/main/command_fuzzer @@
# strace -f ./build/Fuzzer/main/command_fuzzer ./input/1
