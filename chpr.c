#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid, priority;
  
  if(argc <= 2){
    printf(1, "format: chpr pid priority\n");
    exit();
  }
  
  pid = atoi(argv[1]);       // 获取进程号
  priority = atoi(argv[2]);  // 获取新的优先级

  if(chpr(pid, priority) < 0) {
    printf(1, "failed to modify priority.\n");    
  }
  else {
    printf(1, "modify priority successfully.\n");
  }

  exit();
}
