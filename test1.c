#include "types.h"
#include "user.h"

int
main()
{
  float x = 0;
  int i = 0;
  for(x = 0; x < 300000; x += 1) i++;
  printf(1, "x = %d\n", (int)x);
  printf(1, "i = %d\n", i);
  exit();
}
