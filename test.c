#include "types.h"
#include "user.h"
#include "fcntl.h"

volatile int global = 1;  // 测试线程是否共享变量


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
  
  // 测试线程间并行
  for(int i = 0; i < 10; i ++) {  
    printf(1, "thread %d is running.\n", *(int*)arg);
    sleep(100);
  }

  // 测试全局变量、压栈测试
  global = F(15);

  // 测试文件描述符
  write(3, "hello\n", 6);  // 运行后可以在目录下看到新文件
  exit();
}

void boss(void* arg) {
  for(int i = 0; i < 10; i ++) {
    printf(1, "thread %d is running.\n", *(int*)arg);
    sleep(100);
  }
  exit();
}

int
main()
{
  open("tmp", O_RDWR | O_CREATE);  // fd = 3

  int t1 = 1;
  int t2 = 2;
  int tid1 = thread_create(worker, &t1);
  int tid2 = thread_create(boss,   &t2);
  thread_join();
  thread_join();

  printf(1, "thread %d is done\n", tid1);
  printf(1, "thread %d is done\n", tid2);
  printf(1, "global = %d\n", global);
  exit();
}
