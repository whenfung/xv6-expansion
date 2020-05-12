#include "types.h"
#include "user.h"

int n;

void worker(void* arg) {
  exit();
}

int main(int argc, char **argv)
{
  if (argc != 2) exit();
  
  n = atoi(argv[1]);
  printf(1, "执行 %d 个线程切换\n", n);

  uint start = uptime();
  
  for(int i = 0; i < n; i++) {
    thread_create(worker, &i);
    thread_join();
  }

  uint end = uptime();
  printf(1, "花费时间：%d\n", end-start);

  exit();
}
