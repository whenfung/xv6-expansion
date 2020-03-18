#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM 17     // 分配页帧数，可修改

void
mem(void) {
	printf(1, "虚存机制开始测试\n");
  
	char* start = (char*)malloc(4096);       
	char* pre = start;

	for(int i = 1; i < NUM; i++) {
		char* nex = (char*)malloc(4096);
		if (nex == 0)
			goto failed;
		*(char**)pre = nex;
		((int*)pre)[2] = i;
		pre= nex;
	}

	((int*)pre)[2] = NUM;   // 最后一块

  while(1){
    printf(1, "%d : %p \n", ((int*)start)[2], (char*)start);
    if(((int*)start)[2] == NUM) break;
    start = *(char**)start;
  }
	printf(1, "虚存机制测试成功！\n");
	return;
failed:
	printf(1, "虚存机制测试失败！\n");
}

int
main()
{
	mem();
	exit();
}
