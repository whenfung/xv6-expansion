#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
  printf(1, "current CPU id is %d\n", scall());
  exit();
}
