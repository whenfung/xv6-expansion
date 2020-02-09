#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int data[8];

  pid = fork();
  for(int i = 0; i < 2; i ++) {
    for(int j = 0; j < 1024 * 100; j ++)
      for(int k = 0; k < 1024 * 100; k ++)
        data[k % 8] = pid * k;
  }
  printf(1, "%d", data[0]);
  exit();
}
