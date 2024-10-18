### Build
./deploy.sh -b -a
### Run Fuzzer
# export AFL_DEBUG=1
export AFL_SKIP_CPUFREQ=1
export ASAN_OPTIONS=detect_leaks=0,abort_on_error=1,symbolize=0
# export AFL_CUSTOM_MUTATOR_ONLY=1
export AFL_CUSTOM_MUTATOR_LIBRARY=$(pwd)/build/Fuzzer/lib/AFLplusplus-protobuf-mutator/coconut/libcommand_proto_mutator.so
afl-fuzz -i main/test/fuzz/corpus -o output -- ./build/Fuzzer/main/test/fuzz/command_fuzzer @@
