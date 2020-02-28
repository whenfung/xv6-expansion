struct stat;
struct rtcdate;

// system calls
int fork(void);  // 创建进程
int exit(void) __attribute__((noreturn));  // 结束当前进程
int wait(void);   // 等待子进程结束
int pipe(int*);   // 创建管道，并把读和写的 fd 返回到 p
int write(int, const void*, int);  // 从 buf 中写 n 个字节到文件
int read(int, void*, int);  // 从文件中读 n 个字节到 buf
int close(int);    // 关闭打开的 fd
int kill(int);     // 结束 pid 所指进程
int exec(char*, char**);  // 加载并执行一个文件
int open(const char*, int);  // 打开文件，flags 指定读/写模式
int mknod(const char*, short, short); // 创建设备文件
int unlink(const char*);   // 删除文件
int fstat(int fd, struct stat*);  // 返回文件信息
int link(const char*, const char*); // 给 f1 创建一个新名字 f2
int mkdir(const char*); // 创建新的目录
int chdir(const char*);   // 改变当前目录
int dup(int);     // 复制 fd
int getpid(void);  // 获得当前进程 pid
char* sbrk(int);   // 为进程内存空间增加 n 字节
int sleep(int);    // 睡眠 n 秒
int uptime(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
