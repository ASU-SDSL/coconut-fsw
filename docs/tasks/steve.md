# STEVE
Scheduler Task for Executing Vital Events (STEVE) is a core Coconut componenet that schedules misson-critical, time-sensitive jobs. Jobs can either be scheduled once or can be recurring. Jobs can be scheduled using STEVE's api functions. You must provide a job name, job function pointer, and recur/delay time in order to schedule a job.


## Type Definitions

### steve_job_t
The `steve_job_t` type is a struct that represents a steve job. In order to create a steve job, you must provide these fields:  
* name: The name of the job
* execute_time: The tick that the job will be ran
* recur_time: how many ticks until the job recurs. null for non-recurring jobs
* func_ptr: function pointer to the function that steve will run
* arg_data: pointer to the arguments of the job function

### steve_context_t
`steve_context_t` is the type for the global steve context. The struct requires a few fields:  
* jobs: array of `steve_job_t` pointers that acts as the storage for all jobs
* job_count: amount of jobs currently allocated

## Globals
### g_steve_context
`g_steve_context` is the global that represents the global steve context, which contains all jobs. It is of type `steve_context_t`.

## API Functions

<details><summary><big>schedule_recurring_job_ms()</big></summary>

#### Description
> Schedules a job that recurs every `ms_until_recur` milliseconds.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job |
> | job_func_pointer | job_func | pointer to function of job |
> | ms_until_recur | unsigned long | milliseconds until job recurs |

#### Returns
> None

</details>

<details><summary><big>schedule_recurring_job_secs()</big></summary>

#### Description
> Schedules a job that recurs every `secs_until_recur` seconds.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job |
> | job_func_pointer | job_func | pointer to function of job |
> | secs_until_recur | unsigned long | seconds until job recurs |

#### Returns
> None

</details>

<details><summary><big>schedule_recurring_job_mins()</big></summary>

#### Description
> Schedules a job that recurs every `mins_until_recur` minutes.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job |
> | job_func_pointer | job_func | pointer to function of job |
> | mins_until_recur | unsigned long | minutes until job recurs |

#### Returns
> None

</details>

<details><summary><big>schedule_delayed_job_ms()</big></summary>

#### Description
> Schedules a job that will run once after a delay of `ms_delay` milliseconds.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job |
> | job_func_pointer | job_func | pointer to function of job |
> | ms_delay | unsigned long | delay in milliseconds |

#### Returns
> None

</details>

<details><summary><big>schedule_delayed_job_secs()</big></summary>

#### Description
> Schedules a job that will run once after a delay of `secs_delay` seconds.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job |
> | job_func_pointer | job_func | pointer to function of job |
> | secs_delay | unsigned long | delay in seconds |

#### Returns
> None

</details>

<details><summary><big>schedule_delayed_job_mins()</big></summary>

#### Description
> Schedules a job that will run once after a delay of `mins_delay` minutes.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job |
> | job_func_pointer | job_func | pointer to function of job |
> | mins_delay | unsigned long | delay in minutes |

#### Returns
> None

</details>

<details><summary><big>kill_steve_job()</big></summary>

#### Description
> Kills a steve job with the name of `job_name`.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job to kill |


#### Returns
> None

</details>

<details><summary><big>edit_steve_job_recur_time()</big></summary>

#### Description
> Updates the time at which a job recurs with `ms_recur_time` milliseconds.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job to edit |
> | ms_recur_time | unsigned long | new recur time in milliseconds |

#### Returns
> None

</details>

## Internal Functions
### Note: These functions should ***NEVER*** be run outside of `steve.c`

<details><summary><big>initialize_steve()</big></summary>

#### Description
> Initializes steve. Must be run before any other functions.

#### Parameters
> None

#### Returns
> None

</details>

<details><summary><big>find_steve_job()</big></summary>

#### Description
> Finds steve job with given `job_name` and returns the pointer to the steve job struct. The `g_steve_job_mutex` must be taken before using this function.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job_name | const char* | name of job to find |


#### Returns
> steve_job_t*

</details>

<details><summary><big>create_steve_job()</big></summary>

#### Description
> Schedules a new job given by the struct pointer `sr`.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | sr | steve_job_t* | struct pointer for new job |


#### Returns
> None

</details>

<details><summary><big>run_steve_job()</big></summary>

#### Description
> Run the job function given by the struct pointer. The `g_steve_job_mutex` must be taken before using this function.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job | steve_job_t* | struct pointer for job to be run |


#### Returns
> bool

</details>

<details><summary><big>delete_steve_job()</big></summary>

#### Description
> Deletes the given job from the scheduler. The `g_steve_job_mutex` must be taken before using this function.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | job | steve_job_t* | struct pointer for job to be deleted |


#### Returns
> None

</details>

<details><summary><big>cleanup_steve_jobs_list()</big></summary>

#### Description
> Finds null entries in the jobs list and removes them. The `g_steve_job_mutex` must be taken before using this function.

#### Parameters
> None

#### Returns
> None

</details>