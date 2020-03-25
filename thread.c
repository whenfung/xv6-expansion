#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int thread_create(void (*fcn)(void*), void *arg);
int thread_join();

void 
worker(void *arg) {
  *(int*)arg = 2; 
  exit();
}

int
main()
{
  int arg = 1;
  thread_create(worker, (void*)&arg);
  thread_join();
  exit();
}

int thread_create(void (*fcn)(void*), void *arg) {
  void *stack = malloc(2 * PGSIZE);
  if((uint)stack % PGSIZE)
    stack = stack + (PGSIZE - (uint)stack % PGSIZE);
  return clone(fcn, arg, stack); 
}

int thread_join() {
  void *stack;
  int ret = join(&stack);
  // free(stack);
  return ret;
}
