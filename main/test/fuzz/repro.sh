set -e

export ASAN_OPTIONS=detect_leaks=0,abort_on_error=1,symbolize=0
while true; do 
	./build/Fuzzer/main/test/fuzz/command_fuzzer ./output/m_command_fuzzer/crashes/id\:000006\,sig\:11\,src\:000108+000106\,time\:131668061\,execs\:2410314\,op\:libcommand_mutator.so\,pos\:0 
done
