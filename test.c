#include "types.h"
#include "user.h"

void worker(void* arg) {
  printf(1, "thread %d is runing\n", *(int*)arg);
  exit();
}

int
main()
{
  int t = 1;
  thread_create(worker, &t);
  thread_join();
  exit();
}
