#include "types.h"
#include "stat.h"
#include "user.h"

/* Possible states of a thread; */
#define FREE        0x0     // 空闲
#define RUNNING     0x1     // 运行
#define RUNNABLE    0x2     // 就绪

#define STACK_SIZE  8192    // 栈的大小
#define MAX_THREAD  4       // 最多同时运行 4 个栈

typedef struct thread thread_t, *thread_p;
typedef struct mutex mutex_t, *mutex_p;

struct thread {
  int        sp;                /* saved stack pointer */
  char stack[STACK_SIZE];       /* the thread's stack */
  int        state;             /* FREE, RUNNING, RUNNABLE */
};
static thread_t all_thread[MAX_THREAD];  // 线程表, 用于调度
thread_p  current_thread;                // 指向当前线程的指针
thread_p  next_thread;                   // 指向下一个线程的指针
extern void thread_switch(void);         // 切换线程环境

void 
thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule().  it needs a stack so that the first thread_switch() can
  // save thread 0's state.  thread_schedule() won't run the main thread ever
  // again, because its state is set to RUNNING, and thread_schedule() selects
  // a RUNNABLE thread.
  current_thread = &all_thread[0];   // 当前线程设置为指向 main() 的主线程
  current_thread->state = RUNNING;   // 将当前线程的状态设置为 RUNNING
}

static void 
thread_schedule(void)               // 线程调度
{
  thread_p t;

  /* Find another runnable thread. */
  next_thread = 0;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {  // 遍历线程表
    if (t->state == RUNNABLE && t != current_thread) {
      next_thread = t;
      break;
    }
  }

  if (t >= all_thread + MAX_THREAD && current_thread->state == RUNNABLE) {  // 只有一个线程, 即自己
    /* The current thread is the only runnable thread; run it. */
    next_thread = current_thread;
  }

  if (next_thread == 0) {  // 无就绪线程
    printf(2, "thread_schedule: no runnable threads\n");
    exit();
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;       // 设置线程为运行态
    thread_switch();                    // 切换工作环境
  } else
    next_thread = 0;                    // 表示调度器什么都没干
}

void 
thread_create(void (*func)())      // 创建线程
{
  thread_p t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {  // 查找线程控制块
    if (t->state == FREE) break;
  }
  t->sp = (int) (t->stack + STACK_SIZE);   // set sp to the top of the stack
  t->sp -= 4;                              // space for return address
  * (int *) (t->sp) = (int)func;           // push return address on stack
  t->sp -= 32;                             // space for registers that thread_switch expects
  t->state = RUNNABLE;
}

void 
thread_yield(void)       // 放弃 CPU
{
  current_thread->state = RUNNABLE;
  thread_schedule();     // 进入线程调度
}

static void 
mythread(void)
{
  int i;
  printf(1, "my thread running\n");
  for (i = 0; i < 100; i++) {    // 线程切换 100 次
    printf(1, "my thread 0x%x\n", (int) current_thread);
    thread_yield();              // 放弃线程资源
  }
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;  // 释放
  thread_schedule();             // 线程调度
}


int 
main(int argc, char *argv[]) 
{
  thread_init();                // 初始化线程
  thread_create(mythread);      // 创建线程 1
  thread_create(mythread);      // 创建线程 2
  thread_schedule();            // 线程调度
  return 0;
}
