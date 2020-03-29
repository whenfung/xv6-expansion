#include "types.h"
#include "user.h"

#define NTHREAD 4     // 一个进程的最大线程数 (不包括主线程)
#define PGSIZE  4096

struct {
  int pid;
  void* ustack;
  int used;
} threads[NTHREAD];   // TCB 表

// add a TCB to thread table
void add_thread(int* pid, void* ustack) {
  for(int i = 0; i < NTHREAD; i++) {
    if(threads[i].used == 0) {
      threads[i].pid = *pid;
      threads[i].ustack = ustack;
      threads[i].used = 1;
      break;
    }
  }
}

void remove_thread(int* pid) {
  for(int i = 0; i < NTHREAD; i ++) {
    if(threads[i].used && threads[i].pid == *pid) {
      free(threads[i].ustack);   // 释放用户栈
      threads[i].pid = 0;
      threads[i].ustack = 0;
      threads[i].used = 0;
      break;
    }
  }
}

int thread_create(void (*start_routine)(void*), void* arg) {
  // If first time running any threads, initialize thread table with zeros
  static int first = 1;
  if(first) {
    first = 0;
    for(int i = 0; i < NTHREAD; i++) {
      threads[i].pid = 0;
      threads[i].ustack = 0;
      threads[i].used = 0;
    }
  }
  void* stack = malloc(PGSIZE);   // allocate one page for user stack
  int pid = clone(start_routine, arg, stack); // system call for kernel thread
  add_thread(&pid, stack);  // save new thread to thread table
//  printf(1, "arg addr= %x\n", (int)(stack + 4096 - 4));
//  printf(1, "PC = %x\n", *(int*)(stack + 4096 - 8));
  return pid;
}

int thread_join(void) {
  for(int i = 0; i < NTHREAD; i ++) {
    if(threads[i].used == 1) {
      int pid = join();      // 回收子线程
      if(pid > 0) {
        remove_thread(&pid);
        return pid;
      }
    }
  }
  return 0;
}

void printTCB(void) {
  for(int i = 0; i < NTHREAD; i ++)
    printf(1, "TCB %d: %d\n", i, threads[i].used);
}
