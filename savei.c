#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2) {
    printf(1, "format: savei filename temp\n");
    exit();
  }
  uint addrs[13];
  for(int i = 0; i < 13; i ++)
    printf(1, "%d ", addrs[i]);
  printf(1, "\n");

  geti(argv[1], addrs);
  
  for(int i = 0; i < 13; i ++)
    printf(1, "%d ", addrs[i]);
  printf(1, "\n");
  exit();
}
