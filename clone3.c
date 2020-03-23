// clone copies file descriptors, but doesn't share

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "x86.h"

uint newfd = 0;

void 
worker(void *arg_ptr) {
  write(3, "hello\n", 6);
  xchg(&newfd, open("tmp2", O_WRONLY | O_CREATE));
  exit();
}

int
main(int argc, char *argv[])
{
  int pid = getpid();
  void* stack = malloc(4096*2);
  if((uint)stack % 4096)
    stack = stack + (4096 - (uint)stack % 4096);

  int fd = open("tmp", O_CREATE | O_WRONLY);
  int clone_pid = clone(worker, 0, stack);
  while(!newfd);
  printf(1, "TEST PASSED\n");
  exit();
}

