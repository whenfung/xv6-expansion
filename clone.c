// clone and verify that address space is shared

#include "types.h"
#include "stat.h"
#include "user.h"

volatile int global = 1;

void 
worker(void *arg_ptr) {
  global = 5;
  printf(1, "son change global to %d\n", global);
  exit();
}

int
main()
{
  int pid = getpid();
  void* stack = malloc(4096*2);
  if((uint)stack % 4096)
    stack = stack + (4096 - (uint)stack % 4096);

  int clone_pid = clone(worker, 0, stack);
  while(global != 5);
  printf(1, "global = %d\n", global);
  printf(1, "TEST PASSED, %d create %d\n", pid, clone_pid);
  exit();
}


