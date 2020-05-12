#include "types.h"
#include "user.h"

int
main(int argc, char **argv)
{
  if(argc != 2) exit();

  int n = atoi(argv[1]);
  printf(1, "执行 %d 个进程切换\n", n);

  uint start = uptime();
  int pid;
  for(int i = 0; i < n; i++) {
    if((pid = fork()) == 0) {
      exit();
    } else {
      wait();
    }
  }
  uint end = uptime();
  printf(1, "花费时间：%d\n", end-start);

  exit();
}
