#include "types.h"
#include "user.h"

volatile int global = 1;

void worker(void* arg) {
  printf(1, "thread %d is worker.\n", *(int*)arg);
  global = 5;  // verify that address space is shared;
  exit();
}

void boss(void* arg) {
  printf(1, "thread %d is boss.\n", *(int*)arg);
  exit();
}

int
main()
{
  int t1 = 1;
  int t2 = 2;
  thread_create(worker, &t1);
  thread_create(boss,   &t2);
  thread_join();
  thread_join();
  printf(1, "global = %d\n", global);
  exit();
}
