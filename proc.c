#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  if(p->parent == 0) { // TODO: P4 NEW CODE
      p->timeslice = 1; // TODO: P4 NEW CODE
  } // TODO: P4 NEW CODE
  else { // TODO: P4 NEW CODE
      p->timeslice = p->parent->timeslice; // TODO: P4 NEW CODE
  } // P4 NEW CODE

  if(p->pid != 1) { // TODO: P4 NEW CODE
      if(p->pid == 2){
          mycpu()->tail = p;
          mycpu()->head->next = p;
          mycpu()->head->prev = p;
          p->prev = mycpu()->head;
          p->next = mycpu()->head;
      }
      else {
//          struct proc *temp = mycpu()->tail;
          p->prev = mycpu()->tail; // TODO: P4 NEW CODE
          mycpu()->tail->next = p; // TODO: P4 NEW CODE
          mycpu()->tail = p; // TODO: P4 NEW CODE
          mycpu()->head->prev = mycpu()->tail; // TODO: P4 NEW CODE
          p->next = mycpu()->head; // TODO: P4 NEW CODE
      }
  } // TODO: P4 NEW CODE

//    if(p->pid >= 3) { // TODO: P4 NEW CODE
//        cprintf("Curr proc: %d\n", p->pid); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Curr.prev proc: %s\n", p->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Curr.next proc: %s\n", p->next->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Previous proc: %s\n", p->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Prev.prev proc: %s\n", p->prev->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Prev.next proc: %d\n", p->prev->next->pid); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Next proc: %s\n", p->next->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Next.prev proc: %d\n", p->next->prev->pid); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Next.next proc: %s\n", p->next->next->name); // TODO:  REMOVE PRINT STATEMENT
//    }

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  // P4 New   TODO: add tick initialization

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.

  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();
  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }
  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;
  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;
  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);
  safestrcpy(np->name, curproc->name, sizeof(curproc->name));
  np->timeslice = myproc()->timeslice; // New P4 Code
  pid = np->pid;
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;

  // TODO: Trying to remove proc from linkedlist
  curproc->prev->next = curproc->next;
  curproc->next->prev = curproc->prev;
  if(mycpu()->tail == curproc) {
      mycpu()->tail = curproc->prev;
  }
//    if(curproc->pid >= 3) {
//        cprintf("Curr proc: %d\n", curproc->pid); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Curr.prev proc: %s\n", curproc->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Curr.next proc: %s\n", curproc->next->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Previous proc: %s\n", curproc->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Prev.prev proc: %s\n", curproc->prev->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Prev.next proc: %s\n", curproc->prev->next->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Next proc: %s\n", curproc->next->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Next.prev proc: %s\n", curproc->next->prev->name); // TODO:  REMOVE PRINT STATEMENT
//        cprintf("Next.next proc: %s\n", curproc->next->next->name); // TODO:  REMOVE PRINT STATEMENT
//    }
//    struct proc *temp = mycpu()->head;
//    cprintf("Head: "); // TODO:  REMOVE PRINT STATEMENT
//    while(temp != 0) {
//        cprintf("%s->", temp->name); // TODO:  REMOVE PRINT STATEMENT
//        temp = temp->next;
//    }
//    cprintf("\n"); // TODO:  REMOVE PRINT STATEMENT



  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct cpu *c = mycpu();
  struct proc *p = ptable.proc;
  acquire(&ptable.lock);
  c->head = p; // TODO: P4 NEW CODE
  c->tail = p; // TODO: P4 NEW CODE
  c->head->next = c->tail; // TODO: P4 NEW CODE
  c->head->prev = c->tail; // TODO: P4 NEW CODE
  c->tail->next = c->head; // TODO: P4 NEW CODE
  c->tail->prev = c->head; // TODO: P4 NEW CODE
  c->proc = p;
  release(&ptable.lock);
  for(;;) {
      // Enable interrupts on this processor.
      sti();
      acquire(&ptable.lock);
      if (c->proc->state != RUNNABLE) {
      }
          // Switch to chosen process.  It is the process's job
          // to release ptable.lock and then reacquire it
          // before jumping back to us.
      else {
          switchuvm(c->proc);
          c->proc->state = RUNNING;
          int after3 = 0;
          if(p->pid == 3){
              after3 = 1;
          }
//          cprintf("Previous proc: %s\n",p->prev->name); // TODO:  REMOVE PRINT STATEMENT
//          cprintf("Running proc: %s\n",p->name); // TODO:  REMOVE PRINT STATEMENT
//          cprintf("Next proc: %s\n",p->next->name); // TODO:  REMOVE PRINT STATEMENT

          c->proc->switches += 1; // TODO: P4 NEW CODE
          swtch(&(c->scheduler), c->proc->context);

          if(after3 == 1) {
              cprintf("made it here");
          }
          switchkvm();
      }
      c->proc = c->proc->next;
      release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock)) {
      panic("sched ptable.lock");
  }
  if(mycpu()->ncli != 1) {
      panic("sched locks");
  }
  if(p->state == RUNNING) {
      panic("sched running");
  }
  if(readeflags()&FL_IF) {
      panic("sched interruptible");
  }
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);

  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  //TODO: ADD IF CHAN == INIT && SLEEPING {THEN WAKE}
//  if(chan->pid)

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      // TODO: ADD ADDITIONAL CONDITION IF SHOULD BE WOKEN UP
      if (p->state == SLEEPING && p->chan == chan) {
          p->state = RUNNABLE;
          p->switches += 1;
      }
  }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// My new P4 syscall 1
// This sets the time-slice of the specified pid to slice.
// You should check that both pid and slice are valid (slice
// must be > 0); if they are not, return -1.  On successful
// change, return 0.  The time-slice of a process could be
// increased, decreased, or not changed; if pid is the
// currently running process, then its time-slice should be
// immediately changed and applied to this scheduling interval.
// If the process has run for the number ticks it should run
// (or more) according to the new slice value (e.g. it has run
// 6 ticks, but the new time slice value is 4 ticks), you should
// immediately schedule the next process.
int
setslice(int pid, int slice) // TODO: if slice < currently running p, switch
{
  struct proc *p;
  if (slice < 1) {
    return -1;
  }
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid) {
      p->timeslice = slice;
      return 0;
    }
  }
  return -1;
}

// My new P4 syscall 2
// This returns the time slice of the process with pid, which must be a
// positive interger.  If the pid is not valid, it returns -1.
int
getslice(int pid)
{
  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid) {
      return p->timeslice;
    }
  }
  return -1;
}

// My new P4 syscall 3
int
fork2(int slice)
{
  if(slice < 1) {
    return -1;
  }
    int i, pid;
    struct proc *np;
    struct proc *curproc = myproc();
    // Allocate process.
    if((np = allocproc()) == 0){
        return -1;
    }
    // Copy process state from proc.
    if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }
    np->sz = curproc->sz;
    np->parent = curproc;
    np->timeslice = slice; // New P4 code
    *np->tf = *curproc->tf;
    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;
    for(i = 0; i < NOFILE; i++)
        if(curproc->ofile[i])
            np->ofile[i] = filedup(curproc->ofile[i]);
    np->cwd = idup(curproc->cwd);
    safestrcpy(np->name, curproc->name, sizeof(curproc->name));
    pid = np->pid;
    acquire(&ptable.lock);
    np->state = RUNNABLE;
    release(&ptable.lock);

    return pid;
}

// My new P4 syscall 4
int
getpinfo(struct pstat* pstat_getpinfo)
{
  if(pstat_getpinfo == 0) // TODO: Make sure this NULL check works
    return -1;

  struct proc *p;
  int index = 0;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED) {
      pstat_getpinfo->inuse[index] = 0;
    }
    else {
      pstat_getpinfo->inuse[index] = 1; // whether this slot of the process table is in use (1 or 0)
      pstat_getpinfo->pid[index] = p->pid; // process's pid
      pstat_getpinfo->timeslice[index] = p->timeslice; // number of base ticks this process can run in a timeslice
      pstat_getpinfo->compticks[index] = p->compticks; // number of compensation ticks this process has used
      pstat_getpinfo->schedticks[index] = p->schedticks;  // total number of timer ticks this process has been scheduled
      pstat_getpinfo->sleepticks[index] = p->sleepticks; // number of ticks during which this process was blocked
      pstat_getpinfo->switches[index] = p->switches;  // total num times this process has been scheduled
    }
    index++;
  }
  release(&ptable.lock);
  
  return 0;
}
