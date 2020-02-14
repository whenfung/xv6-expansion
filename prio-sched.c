#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int pid;
  printf(1, "This is a demo for priority scheduling!\n");
  
  pid = getpid();
  chpr(pid, 19);    // 父进程优先级设置为 19
  
  // 创建 4 个子进程
  pid = fork();     
  if(pid != 0) {
    chpr(pid, 15);   // set 1st child's prio to 15
    printf(1, "pid%d prio%d.\n", pid, 15);
    
    pid = fork();
    if(pid != 0) {
      chpr(pid, 15);   // set 2nd child's prio to 15
      printf(1, "pid%d prio%d.\n", pid, 15);
      
      pid = fork();
      if(pid != 0) {
        chpr(pid, 5);    // set 3rd child's prio to 5
        printf(1, "pid%d prio%d.\n", pid, 5);
        
        pid = fork();
        if(pid != 0) {
          chpr(pid, 5);    // set 4th child's prio to 5
          printf(1, "pid%d prio%d.\n", pid, 5);
        }
      }
    }
  }
  pid = getpid();
  printf(1, "pid=%d started.\n", pid);

  for(int i = 1; i < 1000000; i ++); // 耗费时间
    
  printf(1, "pid=%d done.\n", pid);

  exit();
}
