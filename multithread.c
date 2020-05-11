#include "types.h"
#include "user.h"

int n;

void worker(void* arg) {
  printf(1, "线程 %d 开始工作\n", *(int*)arg);

  uint start = uptime();
  float x = 1.0;
  for(float i = 0; i < 30/n; i += 1)  // 分配工作量 
    for(float j = 0; j < 100000; j += 0.1) {
      x = x + 0.01 * 1;
    }
  uint end = uptime();
  printf(1, "线程 %d 完成工作, 期间发生了 %d 次时钟中断\n", *(int*)arg, end - start);
  exit();
}

int main(int argc, char **argv)
{
  if (argc != 2) exit();
  
  n = atoi(argv[1]);
  printf(1, "%d 个线程在跑\n", n);

  if( n < 1 || n > 4) exit();

  int arg[4];
  for(int i = 0; i < 4; i ++)
    arg[i] = i+1;

  uint start = uptime();
  
  for(int i = 0; i < n; i++) {
    thread_create(worker, &arg[i]);
  }

  for(int i = 0; i < n; i++) {
    thread_join();
  }

  uint end = uptime();
  printf(1, "花费时间：%d\n", end-start);

  exit();
}
