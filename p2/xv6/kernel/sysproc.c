#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "pstat.h"

//added by Steven Volocyk
volatile int total_reserves = 0;

//added by Steven Volocyk
int
sys_reserve(void){
  int percent = 0;
  if (argint(0, &percent) < 0){
    return -1;
  } 
  if((percent < 0)|| (percent > 100)) {
    return -1; 
  }
  total_reserves += percent;
  if(total_reserves > 200) {
    return -1;
  }
  else {
    proc->rpercent = percent;
    proc->nano_charge = 100;
    proc->reserved = 1;
    return 0;
  }  
}

//added by Steven Volocyk
int
sys_spot(void){
  int bid = 0;
  if (argint(0, &bid) < 0) {
    return -1;
  }
 proc->spot_bid = bid;
 proc->reserved = 0;
 return 0; 
}

//Added by Steven Volocyk
int 
sys_getpinfo(void) {
  struct pstat *p;
  if(argptr(0,(void*)&p, sizeof(*p)) < 0) {
    return -1;
  }
  if (fill_pstat(p)) {
    return 0;
  }
  else{
    return -1; 
  }
}


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

  if(argint(0, &pid) < 0) // get arg at 0, place it in pid.:
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
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
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
