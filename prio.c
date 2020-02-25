#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int 
main(int argc, char* argv[]){
  int n, pid;
  int k;        // 任务量
  double x = 0;

  if( argc < 2 )  n = 1;                   // default value
  else            n = atoi ( argv[1] );    // from command line
  
  if ( n < 0 || n > 20 ) n = 2;

  if ( argc < 3)  k = 1;
  else            k = atoi(argv[2]);

  pid = 0;
  for (int i = 0; i < n; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "%d failed in fork!\n", getpid());
    } 
    else if (pid > 0) {       // parent
      printf(1,"Parent %d creating child %d\n", getpid(), pid);
      wait();
    } 
    else {                    // child 
      printf(1, "Child %d created\n", getpid());
      while(k--)
        for (double j = 0; j < 8000000; j += 0.5){
          x = x + 3.14 * 89.64;   // useless calculations to consume CPU time  
        }
      printf(1, "Child %d finish its work!\n");
      break;
    }
  }
  exit();
}
