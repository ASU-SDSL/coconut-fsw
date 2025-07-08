#include "hello_job.h"
#include "log.h"

// Runs every 10 s and prints one line to the debug log
void hello_job(void *unused) { 
    logln_info("=== HELLO FROM SID - JOB IS RUNNING ==="); 
    logln_info("Hello job called successfully!"); 
}
