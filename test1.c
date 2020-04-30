#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM 56*1024   // 224 MB

void
memtest(void) {
	printf(1, "虚存机制开始测试\n");
  
  char *mem = sbrk(NUM * 4096); 
  for(int i = 0; i < NUM; i++) {
    mem[i*4096] = 'a';
    if((i+1)%1024 == 0) {
      printf(1, "%d\n", i/1024*4);
      printf(1, "%c\n", mem[i*4096]);
    } 
  }


	printf(1, "虚存机制测试成功！\n");
}

int
main()
{
	memtest();
	exit();
}
