#include "types.h"  // XV6 定义的数据类型 
#include "stat.h"   // 文件信息结构体
#include "user.h"   // 用户可用的函数和系统调用

int
main()
{
  char* buf1 = (char*)malloc(4096);
  char* buf2 = (char*)malloc(4096);
  printf(1, "buf1: %p\n", buf1);
  printf(1, "buf2: %p\n", buf2);

  for(int i = 0; i < 26; i ++) {
    buf1[i] = 'a' + i;
    buf2[i] = 'z' - i;
  }
  buf1[26] = '\0';
  printf(1, "%s\n", buf1);

  swapout(buf1);  // 将 buf1 的数据写到 rawdisk 的前 8 个盘块
  swapin(buf2);   // 将 rawdisk 的前 8 个盘块数据读回 buf2
  exit();
}
