#include "types.h"
#include "stat.h"
#include "user.h"

int global = 1;

void 
worker(void *arg_ptr) {
  global = 5;
  exit();
}

int
main(int argc, char *argv[])
{
  int pid = getpid();
  void* stack = malloc(4096*2);
  if((uint)stack % 4096)
    stack = stack + (4096 - (uint)stack % 4096);

  int clone_pid = clone(worker, 0, stack);
  while(global != 5);
  printf(1, "TEST PASSED\n");
  exit();
}


