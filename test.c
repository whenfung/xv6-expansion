#include "types.h"  // XV6 定义的数据类型 
#include "stat.h"   // 文件信息结构体
#include "user.h"   // 用户可用的函数和系统调用

int
main()
{
  char* str1 = (char*)malloc(4096);
  char* str2 = (char*)malloc(4096);

  for(int i = 0; i < 26; i ++) 
    str1[i] = 'a' + i;
  str1[26] = '\0';
  printf(1, "str1: %s\n", str1);

  swapout(str1, 1, 1000);  
  swapin(str2, 1, 1000);   
  
  printf(1, "str2: %s\n", str2);

  exit();
}
