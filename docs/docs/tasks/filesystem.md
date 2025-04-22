# FILESYSTEM
Scheduler Task for Executing Vital Events (STEVE) is a core Coconut componenet that schedules misson-critical, time-sensitive jobs. Jobs can either be scheduled once or can be recurring. Jobs can be scheduled using STEVE's api functions. You must provide a job name, job function pointer, and recur/delay time in order to schedule a job.

The filesystem thread allows Coconut to read and write files from the on-board storage medium. The core of the thread is a FreeRTOS queue that holds read and write operations to the file system. Users push operations on to the queue, and the queue pops them off and performs them.

The file system module that this thread uses is FatFs, an implementation of the FAT file system for embedded systems.


## Type Definitions

### filesystem_queue_operations_t
The `filesystem_queue_operations_t` type is a struct that represents the file system operations placed onto the queue. The struct needs these fields:
* operation_type: The type of the operation
* file_name: The file the user wishes to access
* text_to_write: The text to be written to the file. If the user is reading, this can be ignored
* read_buffer: The buffer that the result from a read operation will be placed into. If the user is writing, this can be ignored

### operation_type_t
`operation_type_t` is an enum that lists the different file operations that can be completed. These are the different operations:
* WRITE
* READ

## Globals
### filesystem_queue
`filesystem_queue` is the global that represents the queue used by the filesystem thread. It is of type `QueueHandle_t`.

## API Functions

<details><summary><big>read_file()</big></summary>

#### Description
> Puts a read operation on to the filesystem queue.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | file_name | const char* | name of the file to read |
> | result_buffer | char* | buffer the file data is read into |

#### Returns
> None

</details>

<details> <summary><big>write_file()</big></summary>

#### Description
> Puts a write operation on to the filesystem queue.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | file_name | const char* | name of the file to read |
> | text_to_write | char* | text to write to file |

#### Returns
> None

</details>

## Internal Functions
### Note: These functions should ***NEVER*** be run outside of `filesystem.c`