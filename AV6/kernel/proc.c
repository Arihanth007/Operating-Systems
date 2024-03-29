#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

struct mlfq_queue mlfq_q;

// newly defined functions
int max(int a, int b)
{
  return (a > b) ? a : b;
}
int min(int a, int b)
{
  return (a < b) ? a : b;
}
// return type of scheduler
int get_scheduler(void)
{
#ifdef DEFAULT
  return RR_NO;
#endif

#ifdef FCFS
  return FCFS_NO;
#endif

#ifdef PBS
  return PBS_NO;
#endif

#ifdef MLFQ
  return MLFQ_NO;
#endif
}
// implementation of setpriority
int setproc_setpriority(int new_priority, int pid)
{
  int found_pid = 0, old_priority = -1;

  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->pid == pid)
    {
      found_pid = 1;
      old_priority = p->static_priority;
      p->static_priority = new_priority;
      p->niceness = 5;
      p->isReset = 1;
    }
    release(&p->lock);
  }

  if (!found_pid)
    printf("Process with PID = %d doesn't exist\n");

  if (get_scheduler() == PBS_NO && new_priority < old_priority)
    yield();

  return old_priority;
}
// add a process to the mlfq queue
void add_proc(int pool, struct proc *p)
{
  // make sure pool is between 0 and 4
  pool = max(0, min(pool, MAX_POOLS - 1));
  int pool_count = mlfq_q.procs_per_pool[pool];

  // add process to queue
  mlfq_q.queue[pool][pool_count] = p;
  mlfq_q.procs_per_pool[pool]++;
  // update process
  p->pool = pool;
  p->age_lastrun = 0;
}
// remove a proccess to the mlfq queue
void remove_proc(struct proc *p)
{
  int index, pool, pool_count;
  pool = p->pool;
  pool_count = mlfq_q.procs_per_pool[pool];
  index = -1;

  // find process index in mlfq queue
  for (int i = 0; i < pool_count; i++)
    if (mlfq_q.queue[pool][i]->pid == p->pid)
      index = i;

  // process not present in queue
  if (index == -1)
    return;

  // remove process and move every process after it by one index
  for (int i = index; i < pool_count - 1; i++)
    mlfq_q.queue[pool][i] = mlfq_q.queue[pool][i + 1];

  // update queue and current process values
  mlfq_q.procs_per_pool[pool]--;
  p->pool = -1;
}

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void proc_mapstacks(pagetable_t kpgtbl)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    char *pa = kalloc();
    if (pa == 0)
      panic("kalloc");
    uint64 va = KSTACK((int)(p - proc));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table at boot time.
void procinit(void)
{
  struct proc *p;

  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");
  for (p = proc; p < &proc[NPROC]; p++)
  {
    initlock(&p->lock, "proc");
    p->kstack = KSTACK((int)(p - proc));
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu *
mycpu(void)
{
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc *
myproc(void)
{
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int allocpid()
{
  int pid;

  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc *
allocproc(void)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == UNUSED)
    {
      goto found;
    }
    else
    {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;
  // added
  p->strace_mask = 0;
  p->ctime = ticks;
  p->etime = 0;
  p->rtime = 0;
  p->stime = 0;
  p->wtime = 0;
  p->rtime_lastrun = 0;
  p->stime_lastrun = 0;
  p->age_lastrun = 0;
  p->scheduled_times = 0;
  p->static_priority = 60;
  p->niceness = 5;
  p->isReset = 0;
  p->pool = 0;
  for (int pool = 0; pool < MAX_POOLS; pool++)
    p->pool_time[pool] = 0;

  // Allocate a trapframe page.
  if ((p->trapframe = (struct trapframe *)kalloc()) == 0)
  {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if (p->pagetable == 0)
  {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  if (p->trapframe)
    kfree((void *)p->trapframe);
  p->trapframe = 0;
  if (p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
  // added
  p->strace_mask = 0;
  p->ctime = 0;
  p->etime = 0;
  p->rtime = 0;
  p->stime = 0;
  p->wtime = 0;
  p->rtime_lastrun = 0;
  p->stime_lastrun = 0;
  p->age_lastrun = 0;
  p->scheduled_times = 0;
  p->static_priority = 0;
  p->niceness = 0;
  p->isReset = 0;
  p->pool = 0;
  for (int pool = 0; pool < MAX_POOLS; pool++)
    p->pool_time[pool] = 0;
}

// Create a user page table for a given process,
// with no user memory, but with trampoline pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if (pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (mappages(pagetable, TRAMPOLINE, PGSIZE,
               (uint64)trampoline, PTE_R | PTE_X) < 0)
  {
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe just below TRAMPOLINE, for trampoline.S.
  if (mappages(pagetable, TRAPFRAME, PGSIZE,
               (uint64)(p->trapframe), PTE_R | PTE_W) < 0)
  {
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// od -t xC initcode
uchar initcode[] = {
    0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
    0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
    0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
    0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
    0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
    0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

// Set up first user process.
void userinit(void)
{
  // for MLFQ
  for (int i = 0; i < MAX_POOLS; i++)
  {
    mlfq_q.max_ticks[i] = (int)1 << i;
    mlfq_q.procs_per_pool[i] = 0;
    for (int j = 0; j < NPROC; j++)
      mlfq_q.queue[i][j] = 0;
  }

  struct proc *p;

  p = allocproc();
  initproc = p;

  // allocate one user page and copy init's instructions
  // and data into it.
  uvminit(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;     // user program counter
  p->trapframe->sp = PGSIZE; // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  // every new process gets added to the queue
  if (get_scheduler() == MLFQ_NO)
  {
    add_proc(0, p);
  }

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n)
{
  uint sz;
  struct proc *p = myproc();

  sz = p->sz;
  if (n > 0)
  {
    if ((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0)
    {
      return -1;
    }
  }
  else if (n < 0)
  {
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if ((np = allocproc()) == 0)
  {
    return -1;
  }

  // Copy user memory from parent to child.
  if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0)
  {
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy strace mask of parent
  np->strace_mask = p->strace_mask;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for (i = 0; i < NOFILE; i++)
    if (p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;

  // every new process gets added to the queue
  if (get_scheduler() == MLFQ_NO)
  {
    add_proc(0, np);
  }

  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc *p)
{
  struct proc *pp;

  for (pp = proc; pp < &proc[NPROC]; pp++)
  {
    if (pp->parent == p)
    {
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status)
{
  struct proc *p = myproc();

  if (p == initproc)
    panic("init exiting");

  // Close all open files.
  for (int fd = 0; fd < NOFILE; fd++)
  {
    if (p->ofile[fd])
    {
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);

  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;
  p->etime = ticks; // update end time

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr)
{
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (np = proc; np < &proc[NPROC]; np++)
    {
      if (np->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        havekids = 1;
        if (np->state == ZOMBIE)
        {
          // Found one.
          pid = np->pid;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                   sizeof(np->xstate)) < 0)
          {
            release(&np->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || p->killed)
    {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); // DOC: wait-sleep
  }
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int waitx(uint64 addr, uint *rtime, uint *wtime)
{
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (np = proc; np < &proc[NPROC]; np++)
    {
      if (np->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        havekids = 1;
        if (np->state == ZOMBIE)
        {
          // Found one.
          pid = np->pid;

          // update time on exiting process
          *rtime = np->rtime;
          *wtime = np->etime - np->ctime - np->rtime;

          if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                   sizeof(np->xstate)) < 0)
          {
            release(&np->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || p->killed)
    {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); // DOC: wait-sleep
  }
}

// updates runtime of a process
void update_time()
{
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == RUNNING)
    {
      p->rtime++;
      p->rtime_lastrun++;
    }
    else if (p->state == SLEEPING)
    {
      p->stime++;
      p->stime_lastrun++;
    }
    else if (p->state == RUNNABLE)
    {
      p->wtime++;
      p->age_lastrun++;
    }
    if (p->state == RUNNING || p->state == RUNNABLE)
    {
      p->pool_time[p->pool]++;
    }

    if (p->isReset)
    {
      p->niceness = 5;
      p->isReset = 0;
    }
    else if (p->rtime_lastrun > 0 || p->stime_lastrun > 0) // denominatior > 0
      p->niceness = (int)(((p->stime_lastrun * 10) / (p->stime_lastrun + p->rtime_lastrun)));

    // only for bonus
    // int is_print_bonus = 1;
    // if (is_print_bonus && get_scheduler() == MLFQ_NO)
    //   if ((p->pid > 2) && (p->state == RUNNING || p->state == RUNNABLE || p->state == SLEEPING))
    //     printf("Process P%d is in Q%d at %d\n", p->pid, p->pool, ticks);

    release(&p->lock);
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  int scheduler_type = get_scheduler();

  c->proc = 0;
  for (;;)
  {
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    if (scheduler_type == RR_NO)
    {
      for (p = proc; p < &proc[NPROC]; p++)
      {
        acquire(&p->lock);

        if (p->state == RUNNABLE)
        {
          // Switch to chosen process.  It is the process's job
          // to release its lock and then reacquire it
          // before jumping back to us.
          p->state = RUNNING;
          c->proc = p;
          swtch(&c->context, &p->context);

          // Process is done running for now.
          // It should have changed its p->state before coming back.
          c->proc = 0;
        }
        release(&p->lock);
      }
    }

    else if (scheduler_type == FCFS_NO)
    {
      struct proc *pick_me = 0;
      uint64 least_ctime = __UINT64_MAX__;
      int found_process = 0;

      // find for process with least creation time
      for (p = proc; p < &proc[NPROC]; p++)
      {
        acquire(&p->lock);
        if ((p->state == RUNNABLE) && (p->ctime < least_ctime))
        {
          least_ctime = p->ctime;
          pick_me = p;
          found_process = 1;
        }
        release(&p->lock);
      }

      if (found_process)
      {
        acquire(&pick_me->lock);

        if (pick_me->state == RUNNABLE)
        {
          pick_me->state = RUNNING;
          c->proc = pick_me;
          swtch(&c->context, &pick_me->context);

          c->proc = 0;
        }
        release(&pick_me->lock);
      }
    }

    else if (scheduler_type == PBS_NO)
    {
      struct proc *pick_me = 0;
      uint64 least_dp = __UINT64_MAX__;
      uint64 least_sch = __UINT64_MAX__;
      uint64 least_ctime = __UINT64_MAX__;
      int found_process = 0, DP = 0;

      // find for process with least creation time
      for (p = proc; p < &proc[NPROC]; p++)
      {
        acquire(&p->lock);
        if (p->state == RUNNABLE)
        {
          // determine dynamic priority
          DP = max(0, min(p->static_priority - p->niceness + 5, 100));

          // Tie conditions
          if (DP > least_dp)
          {
            release(&p->lock);
            continue;
          }
          if (DP < least_dp)
            least_dp = DP;
          else // DP == least_dp
          {
            if (p->scheduled_times > least_sch)
            {
              release(&p->lock);
              continue;
            }
            if (p->scheduled_times < least_sch)
              least_sch = p->scheduled_times;
            else // p->scheduled_times == least_sch
            {
              if (p->ctime > least_ctime)
              {
                release(&p->lock);
                continue;
              }
              least_ctime = p->ctime;
            }
          }

          pick_me = p;
          found_process = 1;
        }
        release(&p->lock);
      }

      if (found_process)
      {
        acquire(&pick_me->lock);

        if (pick_me->state == RUNNABLE)
        {
          pick_me->state = RUNNING;
          pick_me->stime_lastrun = 0;
          pick_me->rtime_lastrun = 0;
          pick_me->scheduled_times++;
          c->proc = pick_me;
          swtch(&c->context, &pick_me->context);

          c->proc = 0;
        }
        release(&pick_me->lock);
      }
    }

    else if (scheduler_type == MLFQ_NO)
    {
      // check for aging to prevent starvation
      // pool 0 doesn't starve
      for (int pool = 1; pool < MAX_POOLS; pool++)
      {
        int pool_count = mlfq_q.procs_per_pool[pool];
        for (int i = 0; i < pool_count; i++)
        {
          if (mlfq_q.queue[pool][i]->age_lastrun > MAX_AGE)
          {
            // printf("AGING AVOIDED\n");
            struct proc *aged_p = mlfq_q.queue[pool][i];
            remove_proc(aged_p);
            add_proc(pool - 1, aged_p);
          }
        }
      }

      // pick the processe to run
      int found_process = 0;
      struct proc *pick_me = 0;

      for (int pool = 0; pool < MAX_POOLS; pool++)
      {
        int pool_count = mlfq_q.procs_per_pool[pool];
        for (int i = 0; i < pool_count; i++)
        {
          if (mlfq_q.queue[pool][i]->state == RUNNABLE)
          {
            pick_me = mlfq_q.queue[pool][i];
            found_process = 1;
            break;
          }
        }
        if (found_process)
          break;
      }

      if (found_process)
      {
        acquire(&pick_me->lock);

        if (pick_me->state == RUNNABLE)
        {
          // for (int i = 0; i < MAX_POOLS; i++)
          // {
          //   printf("q%d : ", i);
          //   for (int j = 0; j < mlfq_q.procs_per_pool[i]; j++)
          //   {
          //     if (mlfq_q.queue[i][j]->state == RUNNABLE || mlfq_q.queue[i][j]->state == RUNNING)
          //       printf("%d(%d) ", mlfq_q.queue[i][j]->pid, mlfq_q.queue[i][j]->rtime_lastrun);
          //   }
          //   printf("\n");
          // }
          // printf("\n");

          pick_me->state = RUNNING;
          pick_me->stime_lastrun = 0;
          pick_me->rtime_lastrun = 0;
          pick_me->age_lastrun = 0;
          pick_me->scheduled_times++;
          c->proc = pick_me;

          swtch(&c->context, &pick_me->context);

          c->proc = 0;
        }
        release(&pick_me->lock);
      }
    }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
  int intena;
  struct proc *p = myproc();

  if (!holding(&p->lock))
    panic("sched p->lock");
  if (mycpu()->noff != 1)
    panic("sched locks");
  if (p->state == RUNNING)
    panic("sched running");
  if (intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void)
{
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (first)
  {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock); // DOC: sleeplock1
  release(lk);

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  release(&p->lock);
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void wakeup(void *chan)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p != myproc())
    {
      acquire(&p->lock);
      if (p->state == SLEEPING && p->chan == chan)
      {
        p->state = RUNNABLE;
      }
      release(&p->lock);
    }
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->pid == pid)
    {
      p->killed = 1;
      if (p->state == SLEEPING)
      {
        // Wake process from sleep().
        p->state = RUNNABLE;
        if (get_scheduler() == MLFQ_NO)
        {
          add_proc(0, p);
        }
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if (user_dst)
  {
    return copyout(p->pagetable, dst, src, len);
  }
  else
  {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if (user_src)
  {
    return copyin(p->pagetable, dst, src, len);
  }
  else
  {
    memmove(dst, (char *)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void)
{
  static char *states[] = {
      [UNUSED] "unused",
      [SLEEPING] "sleep ",
      [RUNNABLE] "runble",
      [RUNNING] "run   ",
      [ZOMBIE] "zombie"};
  struct proc *p;
  char *state;

  printf("\n");
  int ST = get_scheduler();
  if (ST == RR_NO)
    printf("PID\tState  rtime  wtime  nrun\n");
  else if (ST == FCFS_NO)
    printf("PID\tState  rtime  wtime  nrun\n");
  else if (ST == PBS_NO)
    printf("PID  Priority   State  rtime wtime rtime_ls  stime_ls  nrun\n");
  else if (ST == MLFQ_NO)
    printf("PID Priority\tQ State\trtime wtime rtime_ls  stime_ls  nrun\tq0 q1 q2 q3 q4\n");

  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p->state == UNUSED)
      continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";

    if (ST == RR_NO)
      printf("%d\t%s\t%d\t%d\t%d", p->pid, state, p->rtime, p->wtime, p->scheduled_times);
    else if (ST == FCFS_NO)
      printf("%d\t%s\t%d\t%d\t%d", p->pid, state, p->rtime, p->wtime, p->scheduled_times);
    else if (ST == PBS_NO)
      printf("%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d",
             p->pid, p->static_priority, state, p->rtime, p->wtime,
             p->rtime_lastrun, p->stime_lastrun, p->scheduled_times);
    else if (ST == MLFQ_NO)
    {
      int index = -1;
      for (int pool = 0; pool < MAX_POOLS; pool++)
        for (int i = 0; i < mlfq_q.procs_per_pool[pool]; i++)
          if (p->pid == mlfq_q.queue[pool][i]->pid)
            index = i;
      printf("%d\t%d\t%d  %s  %d\t%d\t%d\t%d\t%d\t%d  %d  %d  %d  %d",
             p->pid, index, p->pool, state, p->rtime, p->wtime,
             p->rtime_lastrun, p->stime_lastrun, p->scheduled_times,
             p->pool_time[0], p->pool_time[1], p->pool_time[2], p->pool_time[3], p->pool_time[4]);
    }
    printf("\n");
  }
}
