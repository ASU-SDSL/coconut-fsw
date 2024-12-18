
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "pico/stdlib.h"

#include "spacepacket.h"

extern "C" {
  BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                              const char * const pcName,
                              const configSTACK_DEPTH_TYPE uxStackDepth,
                              void * const pvParameters,
                              UBaseType_t uxPriority,
                              TaskHandle_t * const pxCreatedTask );

  void gse_task(void *pvParameters);

  void steve_task(void *pvParameters);
  
  void command_task(void *pvParameters);
  
  void telemetry_task(void *pvParameters);

  void filesystem_task(void *pvParameters);

  void receive_command_bytes(uint8_t* packet, size_t packet_size);
}

#include "gse.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"

typedef struct fuzzer_input {
  uint8_t *data;
  size_t size;
} fuzzer_input_t;

extern "C" void fuzzer_task(void *input) {
  fuzzer_input_t *fuzzinput = (fuzzer_input_t *)input;
  // Send commands
  receive_command_bytes(fuzzinput->data, fuzzinput->size);
  // Wait
  vTaskDelay(100 / portTICK_PERIOD_MS);
  // End process
  vTaskEndScheduler();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t Size) {
  // Start tasks 
  // TaskHandle_t gse_task_handle;
  // BaseType_t gse_task_status = xTaskCreate(gse_task, 
  //                                     "GSE", 
  //                                     256, 
  //                                     NULL,
  //                                     1,
  //                                     &gse_task_handle);          

  TaskHandle_t scheduler_task_handle;
  BaseType_t scheduler_task_status = xTaskCreate(steve_task, 
                                      "STEVE", 
                                      512, 
                                      NULL, 
                                      1,
                                      &scheduler_task_handle); 

  TaskHandle_t command_task_handle;
  BaseType_t command_task_status = xTaskCreate(command_task,
                                      "COMMAND",
                                      512,
                                      NULL,
                                      1,
                                      &command_task_handle);
  
  TaskHandle_t telemetry_task_handle;
  BaseType_t telemetry_task_status = xTaskCreate(telemetry_task,
                                      "TELEMETRY",
                                      256,
                                      NULL,
                                      1,
                                      &telemetry_task_handle);

  TaskHandle_t filesystem_task_handle;
  BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                      "FILESYSTEM",
                                      512,
                                      NULL,
                                      1,
                                      &filesystem_task_handle);

  // Setup data
  size_t sync_bytes_size = sizeof(COMMAND_SYNC_BYTES)-1;
  size_t fullsize = sync_bytes_size + Size;
  uint8_t *buf = (uint8_t *)pvPortMalloc(fullsize);
  memcpy(buf, COMMAND_SYNC_BYTES, sync_bytes_size);
  memcpy(buf + sync_bytes_size, Data, Size);

  // Setup fuzzer thread
  fuzzer_input_t input;
  input.data = buf;
  input.size = fullsize;                 
  TaskHandle_t fuzzer_task_handle;
  BaseType_t fuzzer_task_status = xTaskCreate(fuzzer_task,
                                      "FUZZER",
                                      256,
                                      &input,
                                      1,
                                      &fuzzer_task_handle);


  // Start the FreeRTOS scheduler
  vTaskStartScheduler();
  // Cleanup
  vPortFree(buf);
  // Kill threads
  // vPortCancelThread(fuzzer_task_handle);
  vTaskDelete(fuzzer_task_handle);

  // vPortCancelThread(scheduler_task_handle);
  vTaskDelete(scheduler_task_handle);

  // vPortCancelThread(command_task_handle);
  vTaskDelete(command_task_handle);

  // vPortCancelThread(telemetry_task_handle);
  vTaskDelete(telemetry_task_handle);

  // vPortCancelThread(filesystem_task_handle);  
  vTaskDelete(filesystem_task_handle);

  // vPortCancelThread(gse_task_handle);
  // vTaskDelete(gse_task_handle);  
  return 0;
}

// int main(int argc, char** argv) {
//   char fopenstr[3] = {'r', 'b', '\x00'};
//   FILE *fileptr = fopen(argv[1], fopenstr);  // Open the file in binary mode
//   struct stat st;
//   lstat(argv[1], &st);
//   size_t filelen = st.st_size;
//   void *buf = malloc(filelen);
//   fread(buf, filelen, 1, fileptr); // Read in the entire file
//   LLVMFuzzerTestOneInput((const uint8_t *)buf, filelen);
//   return 0;
// }


////// First Iteration
// [#0] Id 1, Name: "Scheduler", stopped 0x7ffff784321a in __GI___sigtimedwait (), reason: BREAKPOINT
// [#1] Id 2, Name: "command_fuzzer", stopped 0x7ffff78e57f8 in __GI___clock_nanosleep (), reason: BREAKPOINT
// [#2] Id 3, Name: "GSE", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#3] Id 4, Name: "STEVE", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#4] Id 5, Name: "COMMAND", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#5] Id 6, Name: "TELEMETRY", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#6] Id 7, Name: "FILESYSTEM", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#7] Id 8, Name: "FUZZER", stopped 0x478ca8 in vTaskEndScheduler (), reason: BREAKPOINT
// [#8] Id 9, Name: "command_fuzzer", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#9] Id 10, Name: "Tmr Svc", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: BREAKPOINT
// [#10] Id 11, Name: "Scheduler timer", stopped 0x7ffff78e57f8 in __GI___clock_nanosleep (), reason: BREAKPOINT
// ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
// [#0] 0x478ca8 → vTaskEndScheduler()
// [#1] 0x46002d → fuzzer_task(input=0x7fffffffcc00)
// [#2] 0x47c2a5 → prvWaitForStart(pvParams=0xe084e8)
// [#3] 0x7ffff7894ac3 → start_thread(arg=<optimized out>)
// [#4] 0x7ffff7926850 → clone3()
// ──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
// gef➤  info thread
//   Id   Target Id                                            Frame 
//   1    Thread 0x7ffff7e89780 (LWP 255993) "Scheduler"       0x00007ffff784321a in __GI___sigtimedwait (set=set@entry=0x7fffffffcb18, info=info@entry=0x7fffffffca60, timeout=timeout@entry=0x0) at ../sysdeps/unix/sysv/linux/sigtimedwait.c:61
//   2    Thread 0x7ffff5c00640 (LWP 255996) "command_fuzzer"  0x00007ffff78e57f8 in __GI___clock_nanosleep (clock_id=clock_id@entry=0x0, flags=flags@entry=0x0, req=req@entry=0x7ffff5bffda0, rem=rem@entry=0x7ffff5bffda0) at ../sysdeps/unix/sysv/linux/clock_nanosleep.c:78
//   3    Thread 0x7ffff7e88640 (LWP 255997) "GSE"             __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xdef394) at ./nptl/futex-internal.c:57
//   4    Thread 0x7ffff7e83640 (LWP 255998) "STEVE"           __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe04c04) at ./nptl/futex-internal.c:57
//   5    Thread 0x7ffff7e7e640 (LWP 255999) "COMMAND"         __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe05e94) at ./nptl/futex-internal.c:57
//   6    Thread 0x7ffff7e79640 (LWP 256000) "TELEMETRY"       __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe06924) at ./nptl/futex-internal.c:57
//   7    Thread 0x7ffff7e74640 (LWP 256001) "FILESYSTEM"      __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe07bb4) at ./nptl/futex-internal.c:57
// * 8    Thread 0x7ffff7bff640 (LWP 256002) "FUZZER"          vTaskEndScheduler () at /home/mitchbuntu/Documents/Github/coconut-fsw/lib/FreeRTOS-Kernel/tasks.c:3769
//   9    Thread 0x7ffff7bfa640 (LWP 256003) "command_fuzzer"  __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe08cd0) at ./nptl/futex-internal.c:57
//   10   Thread 0x7ffff7bf5640 (LWP 256004) "Tmr Svc"         __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe098f4) at ./nptl/futex-internal.c:57
//   11   Thread 0x7ffff5200640 (LWP 256005) "Scheduler timer" 0x00007ffff78e57f8 in __GI___clock_nanosleep (clock_id=clock_id@entry=0x0, flags=flags@entry=0x0, req=req@entry=0x7ffff51ffdd0, rem=rem@entry=0x0) at ../sysdeps/unix/sysv/linux/clock_nanosleep.c:78A


////// Second iteration
// [#0] Id 1, Name: "Scheduler", stopped 0x7ffff784321a in __GI___sigtimedwait (), reason: SIGINT
// [#1] Id 2, Name: "command_fuzzer", stopped 0x7ffff78e57f8 in __GI___clock_nanosleep (), reason: SIGINT
// [#2] Id 12, Name: "GSE", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#3] Id 13, Name: "Scheduler", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#4] Id 14, Name: "Scheduler", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#5] Id 15, Name: "COMMAND", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#6] Id 16, Name: "Scheduler", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#7] Id 17, Name: "Scheduler", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#8] Id 18, Name: "Scheduler", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#9] Id 19, Name: "Tmr Svc", stopped 0x7ffff7891117 in __futex_abstimed_wait_common64 (), reason: SIGINT
// [#10] Id 20, Name: "Scheduler timer", stopped 0x7ffff78e57f8 in __GI___clock_nanosleep (), reason: SIGINT
// ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
// [#0] 0x7ffff784321a → __GI___sigtimedwait(set=0x7fffffffca08, info=0x7fffffffc950, timeout=0x0)
// [#1] 0x7ffff78428ac → __GI___sigwait(set=0x7fffffffca08, sig=0x7fffffffca8c)
// [#2] 0x47c3bf → xPortStartScheduler()
// [#3] 0x478b88 → vTaskStartScheduler()
// [#4] 0x4601de → LLVMFuzzerTestOneInput(Data=0xe087a0 "\n\261\241\367\377\177", Size=0x1)
// [#5] 0x423ac4 → fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long)()
// [#6] 0x422efe → fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long, bool, fuzzer::InputInfo*, bool, bool*)()
// [#7] 0x42504c → fuzzer::Fuzzer::ReadAndExecuteSeedCorpora(std::vector<fuzzer::SizedFile, fuzzer::fuzzer_allocator<fuzzer::SizedFile> >&)()
// [#8] 0x42544c → fuzzer::Fuzzer::Loop(std::vector<fuzzer::SizedFile, fuzzer::fuzzer_allocator<fuzzer::SizedFile> >&)()
// [#9] 0x41385c → fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long))()
// ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
// gef➤  info thread
//   Id   Target Id                                            Frame 
// * 1    Thread 0x7ffff7e89780 (LWP 255993) "Scheduler"       0x00007ffff784321a in __GI___sigtimedwait (set=set@entry=0x7fffffffca08, info=info@entry=0x7fffffffc950, timeout=timeout@entry=0x0) at ../sysdeps/unix/sysv/linux/sigtimedwait.c:61
//   2    Thread 0x7ffff5c00640 (LWP 255996) "command_fuzzer"  0x00007ffff78e57f8 in __GI___clock_nanosleep (clock_id=clock_id@entry=0x0, flags=flags@entry=0x0, req=req@entry=0x7ffff5bffda0, rem=rem@entry=0x7ffff5bffda0) at ../sysdeps/unix/sysv/linux/clock_nanosleep.c:78
//   12   Thread 0x7ffff7e88640 (LWP 257004) "GSE"             __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xdef394) at ./nptl/futex-internal.c:57
//   13   Thread 0x7ffff7e74640 (LWP 257005) "Scheduler"       __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe07bb0) at ./nptl/futex-internal.c:57
//   14   Thread 0x7ffff7e79640 (LWP 257006) "Scheduler"       __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe06920) at ./nptl/futex-internal.c:57
//   15   Thread 0x7ffff7e7e640 (LWP 257007) "COMMAND"         __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe05e94) at ./nptl/futex-internal.c:57
//   16   Thread 0x7ffff7e83640 (LWP 257008) "Scheduler"       __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe04c00) at ./nptl/futex-internal.c:57
//   17   Thread 0x7ffff7bff640 (LWP 257009) "Scheduler"       __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe08640) at ./nptl/futex-internal.c:57
//   18   Thread 0x7ffff7bfa640 (LWP 257010) "Scheduler"       __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe098f0) at ./nptl/futex-internal.c:57
//   19   Thread 0x7ffff7bf5640 (LWP 257011) "Tmr Svc"         __futex_abstimed_wait_common64 (private=0x0, cancel=0x1, abstime=0x0, op=0x189, expected=0x0, futex_word=0xe0e924) at ./nptl/futex-internal.c:57
//   20   Thread 0x7ffff5200640 (LWP 257012) "Scheduler timer" 0x00007ffff78e57f8 in __GI___clock_nanosleep (clock_id=clock_id@entry=0x0, flags=flags@entry=0x0, req=req@entry=0x7ffff51ffdd0, rem=rem@entry=0x0) at ../sysdeps/unix/sysv/linux/clock_nanosleep.c:78