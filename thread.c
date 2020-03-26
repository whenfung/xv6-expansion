#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int thread_create(void (*fcn)(void*), void *arg);
int thread_join();

void 
worker(void *arg) {
  printf(1, "线程 %d 正在工作\n", *(int*)arg);
  *(int*)arg = 2; 
  exit();
}

int
main()
{
  int t1 = 1;
  int t2 = 2;
  thread_create(worker, (void*)&t1);
  thread_create(worker, (void*)&t2);
  thread_join();
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
