#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
  clone(0, 0, 0);
  join(0);
  exit();
}
