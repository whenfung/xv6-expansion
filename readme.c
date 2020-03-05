#include "types.h"  // XV6 定义的数据类型 
#include "stat.h"   // 文件信息结构体
#include "user.h"   // 用户可用的函数和系统调用
#include "fcntl.h"  // 打开文件的方式

char buf[8192];

void readme() {
  int fd = open("README.md", O_RDONLY);   // fd >= 0 表示创建成功

  struct stat st;
  fstat(fd, &st);       // 读取文件信息 
  printf(1, "dev: %d\n", st.dev);
  printf(1, "ino: %d\n", st.ino);
  printf(1, "nlink: %d\n", st.nlink);
  printf(1, "type: %d\n", st.type);
  printf(1, "size: %d\n", st.size);

  read(fd, buf, 2000);
  printf(1, "content is below:\n%s", buf);
  close(fd);
}

int
main()
{ 
  // readme();
  
  printf(1, "addr: %p\n", buf);
  readd(buf);
  printf(1, "%s\n", buf);
  exit();
}
