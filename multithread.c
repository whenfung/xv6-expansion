#include "types.h"
#include "user.h"

int n;

void worker(void* arg) {
  printf(1, "线程 %d 开始工作\n", *(int*)arg);

  uint start = uptime();

  int a = 0;
  int b = 0; 
  int c = 0;
  for(int i = 0; i < 60/n; i ++) {  // 分配工作量 
    for(int j = 0; j < 1000; j ++) {
      for(int k = 0; k < 100000; k++) {
        c++;
        if( c > 100000 ) {
          c = c - 100000;
          b++;
        }
        if( b > 100000) {
          b = b - 100000;
          a ++;
        }
      }
    }
  }
  printf(1,"a = %d, b = %d, c = %d\n", a, b, c);
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
