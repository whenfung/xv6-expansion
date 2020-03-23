#include "types.h"
#include "stat.h"
#include "user.h"

volatile int arg = 55;
volatile int global = 1;

void 
worker(void *arg_ptr) {
  int tmp = *(int*)arg_ptr;
  *(int*)arg_ptr = 1;
  global = tmp;
  exit();
}

int
main(int argc, char *argv[])
{
  int pid = getpid();
  void* stack = malloc(4096*2);
  if((uint)stack % 4096)
    stack = stack + (4096 - (uint)stack % 4096);

  int clone_pid = clone(worker, (void*)&arg, stack);
  while(global != 55);
  printf(1, "TEST PASSED\n");
  exit();
}


