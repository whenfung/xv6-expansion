#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  sh_var_write(0);
  int pid = fork();
  
  for(int i = 0; i < 100000; i ++)
    sh_var_write(sh_var_read() + 1);
  
  printf(1, "sum = %d\n", sh_var_read());
  
  if(pid > 0)  wait();
  
  exit();
}
