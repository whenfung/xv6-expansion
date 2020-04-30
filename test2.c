#include "types.h"
#include "user.h"

void worker(void* arg) {
  // 线程工作
  float x = 1.0;
  for(float i = 0; i < 120; i+=1) 
    for(float j = 0; j < 100000; j+=0.1) {
      x = x*1*3.14*2.18*1.99;
      x = 1.0;
    }
  printf(1, "%d 线程完成工作\n", *(int*)arg);
  exit();
}

int
main()
{
  uint start = uptime();
  int t1=1;
  thread_create(worker, &t1);
  thread_join();
  uint end = uptime();
  printf(1, "%d\n", end-start);

  exit();
}
