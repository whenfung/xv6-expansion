#include "types.h"
#include "user.h"

int
main()
{
  thread_create(0, 0);
  thread_join();
  exit();
}
