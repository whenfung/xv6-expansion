#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM 1024*1024*1024   // 1 GB

void
memtest(void) {
	printf(1, "虚存机制开始测试\n");
  
  char *mem = sbrk(NUM); 
  for(int i = 0; i < 1024; i++) {
    mem[i*1024*1024] = 'a';
    printf(1, "%d : %c\n", i, mem[i*1024*1024]);
  }

	printf(1, "虚存机制测试成功！\n");
}

int
main()
{
	memtest();
	exit();
}
