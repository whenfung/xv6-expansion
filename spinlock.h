// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint pcs[10];      // The call stack (an array of program counters)
                     // that locked the lock.
};

int sh_var;          // 全局共享变量

#define SEM_MAX_NUM 128          // 信号量总数
struct sem{
  struct spinlock lock;          // 内核自旋锁
  int resource_count;            // 资源计数
  int len;                       // 阻塞进程数量
  int allocated;                 // 是否被分配：1 已分配，0 未分配
  int procs[];                   // 阻塞进程号
};

extern int   sem_used_count;     // 当前在用信号量数目
extern struct sem sems[SEM_MAX_NUM]; // 系统信号量上限
