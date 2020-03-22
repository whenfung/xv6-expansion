#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];   // 盘块缓冲区

int
main(int argc, char *argv[])
{
  if(argc < 2) {
    printf(1, "format: savei filename temp\n");
    exit();
  }
  uint addrs[13];
  int fd;
  
  // 找出删除文件的索引信息 
  fd = open("temp", O_RDONLY);
  read(fd, addrs, sizeof(addrs));
  close(fd);

  // 根据索引信息读取相应的盘块
  fd = open(argv[1], O_CREATE | O_RDWR);
  for(int i = 0; i < 13 && addrs[i] != 0; i++) {
    recoverb(addrs[0], buf);   // 读取数据到缓存块 buf 中
    write(fd, buf, 512);       // 写到新文件
  }
  close(fd);
  exit();
}
