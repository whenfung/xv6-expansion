#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int pid;
  int data[8];
  printf(1, "This is a demo for priority scheduling!\n");

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
          printf(1, "pid%d prio%d.\n", pid, 15);
        }
      }
    }
  }
  pid = getpid();
  printf(1, "pid=%d started.\n", pid);
  for(int i = 0; i < 2; i ++) {
    printf(1, "pid=%d runing\n", pid);
    for(int j = 0; j < 1024*100; j++)
      for(int k = 0; k < 1024; k ++)  
        data[k%8] = pid * k;
  }
  printf(1, "%d \n", data[pid]);

  exit();
}
