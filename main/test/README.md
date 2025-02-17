## Unit Tests
The unit tests are implemented and CMock and can be added by creating a function with the prefix *_job (for example) inside the `unit/steve/steve_test_unittest.c` file. These can then be ran automatically from the `steve_test_unittest` binary after building with `TBD` command in the in the root project directory. The only dependency for this build type is Ruby (which CMock uses to generate mock files).

## Fuzzing
The fuzzer is implemented with `AFL++` and has a couple of dependenices. First, you must [build and install](https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/INSTALL.md) AFL++ (version 4.30c recommended):
```bash
sudo apt-get update
sudo apt-get install -y build-essential python3-dev automake cmake git flex bison libglib2.0-dev libpixman-1-dev python3-setuptools cargo libgtk-3-dev
sudo apt-get install -y lld-14 llvm-14 llvm-14-dev clang-14 || sudo apt-get install -y lld llvm llvm-dev clang
sudo apt-get install -y gcc-$(gcc --version|head -n1|sed 's/\..*//'|sed 's/.* //')-plugin-dev libstdc++-$(gcc --version|head -n1|sed 's/\..*//'|sed 's/.* //')-dev
sudo apt-get install -y ninja-build # for QEMU mode
sudo apt-get install -y cpio libcapstone-dev # for Nyx mode
sudo apt-get install -y wget curl # for Frida mode
sudo apt-get install -y python3-pip # for Unicorn mode
git clone https://github.com/AFLplusplus/AFLplusplus
cd AFLplusplus
git checkout v4.30c
make PERFORMANCE=1 distrib
sudo make install
```

Then it is recommended to install [AFL_runner](https://github.com/0xricksanchez/AFL_Runner) to easily parallelize fuzzer instances across multiple cores. Make sure you have the latest version of rust installed through rustup and run the following command: 
```bash
cargo install afl_runner
```
There is also a standard `afl-fuzz` command commented-out in the `main/test/fuzz/fuzz.sh` run script if you don't want to parallelize the fuzzer. Either way, you should peek in this script to tweak performance options and adjust the core count on alfr.

Once everything is installed, just run the previously-mentioned `main/test/fuzz/fuzz.sh` run script from the root source dir and start finding bugs.

Keep in mind the fuzzer can break often and has to be maintained when new features are added to the codebase. Checkout the `fuzzing` branch to try the last confirmed working version of the fuzzer.
