#include "types.h"
#include "user.h"
#include "fcntl.h"

volatile int global = 1;


int F(int n)   //  斐波那契数列, 测试用户栈
{
	if(n < 0)
		printf(1, "请输入一个正整数\n");
	else if(n == 1 || n == 2)
		return 1;
	else {
		return F(n - 1) + F(n - 2);
	}
	return 0;
}

void worker(void* arg) {
//  printf(1, "arg addr = %x\n", (int)&arg);
  printf(1, "thread %d is worker.\n", *(int*)arg);
  
  // 测试全局变量、压栈测试
  global = F(15);
  
  // 测试文件描述符
  write(3, "hello\n", 6);  // 运行后可以在目录下看到新文件

  exit();
}

void boss(void* arg) {
  printf(1, "thread %d is boss.\n", *(int*)arg);
  exit();
}

int
main()
{
  int t = 1;
  open("tmp", O_RDWR | O_CREATE);
  int pid = thread_create(worker, &t);
  thread_join();
  printf(1, "thread id = %d\n", pid);
  printf(1, "global = %d\n", global);
  exit();
}
