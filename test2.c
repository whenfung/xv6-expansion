#include "types.h"
#include "user.h"

void worker(void* arg) {
  uint start = uptime();
  
  // 线程工作
  float x = 1.0;
  for(float i = 0; i < 15; i+=1) 
    for(float j = 0; j < 100000; j+=0.1) {
      x = x*1*3.14*2.18*1.99;
      x = 1.0;
    }

  uint end = uptime();
  printf(1, "%d 线程完成工作 %d\n", *(int*)arg, end - start);
  exit();
}

int
main()
{
  uint start = uptime();
  int t1=1;
  int t2=2;
  thread_create(worker, &t1);
  thread_create(worker, &t2);
  thread_join();
  thread_join();
  uint end = uptime();
  printf(1, "花费时间：%d\n", end-start);

  exit();
}
