#include "types.h"
#include "stat.h"
#include "user.h"

void
mem(void) {
	printf(1, "请求调页功能开始测试\n");
  
  // 分配内存并写内存
	char* start = (char*)malloc(4096);     // 分配 4K
	char* pre = start;

	for(int i = 1; i <= 10; i++) {   // 分配 10 次 4K
		char* nex = (char*)malloc(4096);
		if (nex == 0)
			goto failed;
		*(char**)pre = nex;            // 将内存段串成链表
		((int*)pre)[2] = i;            // 分配顺序
		pre= nex;
	}

	((int*)pre)[2] = 11;             // 给第 11 块内存段标号

  // 输出相关信息
  while(1){
    printf(1, "%d : %p \n", ((int*)start)[2], (char*)start);
    if(((int*)start)[2] == 11) break;
    start = *(char**)start;
  }
	printf(1, "请求调页机制测试成功！\n");
	return;
failed:
	printf(1, "请求调页机制测试失败！\n");
}

int
main()
{
	mem();
  exit();
}
