# delete old logs
for file in ./crashes_min/*.txt; do
  rm $file
done

for file in ./crashes_min/*; do
    echo $file
    ./build/Fuzzer/main/test/fuzz/command_fuzzer "$file" > "file.txt"
done
