#include "types.h"
#include "stat.h"
#include "user.h"

void 
worker(void *arg) {
  *(int*)arg = 2; 
  exit();
}

int
main()
{
  void* stack = malloc(4096*2);
  if((uint)stack % 4096)
    stack += (4096 - (uint)stack % 4096);

  int arg = 1;
  thread_create(worker, (void*)&arg, stack);
  exit();
}


