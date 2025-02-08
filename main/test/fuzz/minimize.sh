# backup old crashes
rm -rf crashes_old
mkdir crashes_old
mv -f crashes crashes_old

# move all fuzzer crashes to seperate dir
mkdir crashes
cp -rf ./output/sub_*/crashes/* ./crashes

# cmin time
mkdir crashes_min
afl-cmin -A -i crashes -o crashes_min -T 8 -- ./build/Fuzzer/main/test/fuzz/command_fuzzer @@ 

