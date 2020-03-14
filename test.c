#include "types.h"
#include "stat.h"
#include "user.h"

void
mem(void) {
	printf(1, "分页式开始测试\n");
  
  // 测试只分配不写时剩余物理页帧的变化 
  char *start =(char*) malloc(4096*5);

  // 测试分配且写时剩余物理页帧的变化，并查看其线性地址是否连续。
	start = (char*)malloc(4096);       
	char* pre = start;

	for(int i = 1; i <= 10; i++) {
		char* nex = (char*)malloc(4096);
		if (nex == 0)
			goto failed;
		*(char**)pre = nex;
		((int*)pre)[2] = i;
		pre= nex;
	}

	((int*)pre)[2] = 11;   // 第 11 块

  while(1){
    printf(1, "%d : %p \n", ((int*)start)[2], (char*)start);
    if(((int*)start)[2] == 11) break;
    start = *(char**)start;
  }
	printf(1, "请求分页机制测试成功！\n");
	return;
failed:
	printf(1, "请求分页机制测试失败！\n");
}

int
main()
{
	mem();
	printf(1, "退出\n");
  return 0;
}
