# AV6 - Enhanced XV6

## Running the OS

Run the OS bu typing the command where `name` can be FCFS, PBS or MLFQ. For round robin, simply omit `SCHEDULER=<name>`. Thus RR is the default scheduler.

```bash
make clean qemu SCHEDULER=<name> CPUS=<number>
```

## Overview

-   `strace` - a system call that traces other system calls and displays useful information about them.
-   `waitx` - keeps track of the runtime and waitime of a process.
-   Various schedulers such as FCFS (first come first served), PBS (priority based scheduler) and MLFQ (multilevel feedback queue scheduling) implemented.
-   We also time various criteria that help understand a process's lifecycle.
-   `schedulertest` performs various processes and times them for the different types of schedulers.

## Task 1 - strace

```bash
strace mask command [args]
```

-   `mask` tells us which process to trace (it is given to us in binary).
-   The syscall prints information relevant to the command.
-   The structure - proc is modified to hold the value mask in `proc.h`.
-   We define strace in `syscall.h` and have the logic for it in `syscall.c`.
-   We also handle the child processes by setting the mask of the child equal to the parent in `proc.c`.

## Task 2 - scheduling

If the scheduler isn't specified then it is taken by default to be round robin (RR). We can set the scheduler by using the `SCHEDULER` flag.

### FCFS

First come first served scheduling algorithm takes the processes in the order they came in and executes each one till completion before moving on to the next process.

The way the scheduler decides which process came first is by looking at the creation time of each proces.

For this we store the creation time of every process as a new variable called `ctime` in the proc structure and assign it in the `allocproc` fucntion when the process starts.

This algortihm is preemtive which means that the clock interrupt is disabled in `usertrap` and `kerneltrap`, thus being able to complete a process before moving on to the next one. This modification was done in `trap.c`.

### PBS

Priority based scheduler picks the process with highest priority. We calculate and assign dynamic priority to every process during runtime.

The information we have before hand are the niceness values (set to 5 by default) and the static priority (set to 60 by default) which we store in the proc structure as `niceness` and `static_priority` respectively.

We need additional information such as the number of times a process has been scheduled `scheduled_times`, sleep time since its last run `stime_lastrun`, and runtime since its last run `rtime_lastrun` which we also store in the proc structure.

The syscall `setpriority` can change the value of static priority of a process and resets the niceness values. Thus we update the dynamic priority at every run. Niceness is updated in `update_time` function in `proc.c`.

If multiple processes have the same priority then the tie is broken by considering the process that has been scheduled fewer times. If that were a tie too then we look at the process with the older creation time.

This algortihm is preemtive which means that the clock interrupt is disabled in `usertrap` and `kerneltrap`. This modification was done in `trap.c`.

### MLFQ

Multilevel feedback queue algorithm schedules processes based on their priority.

Consider 5 different pools of priority, 0 being the most important and 4 being the least important. Each of these pools contains a queue of processes that are executed for a fixed number of ticks assigned to that pool and then switches to the next process by putting the current one at the end of the queue. A round robin scheduler is used for the processes in the 4th queue.

To store all this information, a new structure `mlfq_queue` is created in `proc.h` that contains `queue` - queue of pools and processes in them, `max_ticks` - defines how many ticks the process in that pool can run for, and `procs_per_pool` - keeps track of how many processes are in each pool. Additionally we store which pool a process is in and how much time it has spent in each pool using `pool` and `pool_time` in struct proc.

We have created two functions - `add_proc` and `remove_proc` in `proc.c` that adds a process and removes a process from the queue.

Further, to prevent starvation we have a `MAX_AGE` that a porcess can reach age to before its priority starts to increse. I have used taken 25 to be the max age. For this we have added `age_lastrun` to proc.

In our `usertrap` and `kernertrap` functions we check to see if priority need to be changed based on how much time the process spent in the pool and make appropriate changes before yielding.

A process voluntarily giving up control of the CPU (for doing I/O work, etc) is smart since it won't be needing the CPU time. By doing this, the process gets placed at the end of the queue and can resume its CPU intense work after its done with I/O upto the number of ticks allowed in that pool. This way they process can maximise its CPU utilisation. Note that by giving up the CPU, it doesn't register a tick, hence it is staying in the same priority pool.

### Analysis

Analysis was done with 1 CPU for all scheduling tasks to maintain interpretability using `schedulertest`.

| Scheduler | Avg Run Time | Avg Wait Time |
| --------- | ------------ | ------------- |
| RR        | 17           | 170           |
| FCFS      | 35           | 163           |
| PBS       | 18           | 226           |
| MLFQ      | 17           | 168           |

## Task 3 - procdump

`procdump` is present in `proc.c` and has been modified to display relevant information for the various types of schedulers.

## Bonus

The output of MLFQ was printed using the `update_time` function in `proc.c` and saved the output to `outputforbonus.txt`. This data was then graphed to obtain `MLFQ_PLOT.png`.
