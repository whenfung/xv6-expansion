#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 2) {
    printf(1, "format: savei filename temp\n");
    exit();
  }

  uint addrs[13];        // 存储索引信息的数组
  geti(argv[1], addrs);  // 获取索引信息
  
  int fd = open("temp", O_CREATE | O_RDWR); // 将索引信息写到 temp 文件上
  write(fd, addrs, sizeof(addrs));
  close(fd);
  exit();
}
