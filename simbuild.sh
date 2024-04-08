cmake -S . -B build/ -DCMAKE_C_COMPILER=$(which gcc-9) -DCMAKE_BUILD_TYPE=Release
cmake --build build