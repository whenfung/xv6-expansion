#include "types.h"  // XV6 定义的数据类型 
#include "stat.h"   // 文件信息结构体
#include "user.h"   // 用户可用的函数和系统调用

int
main()
{
  char* str1 = (char*)malloc(4096);
  printf(1, "进程空间地址: %p\n", str1);
  char* str2 = (char*)malloc(4096);
  printf(1, "进程空间地址: %p\n", str2);

  for(int i = 0; i < 8; i ++) 
    str1[i*512] = 'a' + i;
  
  printf(1,"str1: ");
  for(int i = 0; i < 8; i ++)
    printf(1, "%c", str1[i*512]);
  
  uint blockno = swapout(str1);  
  swapin(str2, blockno);   
  
  printf(1, "\nstr2: ");
  for(int i = 0; i < 8; i ++)
    printf(1, "%c", str2[i*512]);
  printf(1, "\n");

  exit();
}
