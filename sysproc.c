#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  myproc()->startsleepticks = ticks;
  myproc()->sleepfor = n; // NEW P4 CODE: Only for debugging purposes
  myproc()->issleeping = 1; // NEW P4 CODE
  myproc()->wakeuptime = ticks0 + n; // NEW P4 CODE
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// My new P4 syscall 1
int
sys_setslice(void)
{
  int pid_setslice;
  int slice_setslice;
  if((argint(0, &pid_setslice) < 0) || (argint(1, &slice_setslice) < 0))
    return -1;
  if(slice_setslice <= 0)
    return -1;

  return setslice(pid_setslice, slice_setslice);
}

// My new P4 syscall 2
int
sys_getslice(void)
{
  int pid_getslice;
  if(argint(0, &pid_getslice) < 0)
    return -1;
  if(pid_getslice <= 0)
    return -1;
    
  return getslice(pid_getslice);
}

// My new P4 syscall 3
int
sys_fork2(void)
{
  int slice_fork2;
  if(argint(0, &slice_fork2) < 0)
    return -1;
  if(slice_fork2 <= 0)
    return -1;

  return fork2(slice_fork2);
}

// My new P4 syscall 4
int
sys_getpinfo(void)
{
  struct pstat *pstat_getpinfo;

  if(argptr(0, (void*)&pstat_getpinfo, sizeof(*pstat_getpinfo)) < 0)
    return -1;
  
  return getpinfo(pstat_getpinfo);
}
