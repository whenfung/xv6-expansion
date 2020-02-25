#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
  fork();
  fork();
  while(1); 
  exit();
}
