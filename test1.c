#include "types.h"
#include "user.h"

int
main()
{
  int i = 0;
  for(int x = 0; x < 300000; x += 1) i++;
  printf(1, "i = %d\n", i);
  exit();
}
