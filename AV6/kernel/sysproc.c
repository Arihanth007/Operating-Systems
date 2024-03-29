#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;

  // get 1st, 2nd and 3rd arguments
  if (argaddr(0, &addr) < 0)
    return -1;
  if (argaddr(1, &addr1) < 0)
    return -1;
  if (argaddr(2, &addr2) < 0)
    return -1;

  int ret = waitx(addr, &wtime, &rtime);

  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;

  return ret;
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_strace(void)
{
  int strace_mask;
  if (argint(0, &strace_mask) < 0)
    return -1;

  int FAIL = -1, SUCCESS = 0;
  struct proc *p = myproc();

  if (p->strace_mask > 0)
    return FAIL;

  p->strace_mask = strace_mask;
  return SUCCESS;
}

uint64
sys_setpriority(void)
{
  int niceness, pid;
  if (argint(0, &niceness) < 0)
    return -1;
  if (argint(1, &pid) < 0)
    return -1;

  setproc_setpriority(niceness, pid);

  return 0;
}
