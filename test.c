#include "types.h"
#include "user.h"

void worker(void* arg) {
  printf(1, "thread %d is runing\n", *(int*)arg);
  exit();
}

int
main()
{
  int t1 = 1;
  int t2 = 2;
  int t3 = 3;
  thread_create(worker, &t1);
  thread_create(worker, &t2);
  thread_create(worker, &t3);
  thread_join();
  thread_join();
  thread_join();
  exit();
}
