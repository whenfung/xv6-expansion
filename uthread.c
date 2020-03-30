#include "types.h"
#include "user.h"

#define NTHREAD 4     // 一个进程的最大线程数 (不包括主线程)
#define PGSIZE  4096

struct {
  int tid;
  void* ustack;
  int used;
} threads[NTHREAD];   // TCB 表

// add a TCB to thread table
void add_thread(int* tid, void* ustack) {
  for(int i = 0; i < NTHREAD; i++) {
    if(threads[i].used == 0) {
      threads[i].tid = *tid;
      threads[i].ustack = ustack;
      threads[i].used = 1;
      break;
    }
  }
}

void remove_thread(int* tid) {
  for(int i = 0; i < NTHREAD; i ++) {
    if(threads[i].used && threads[i].tid == *tid) {
      free(threads[i].ustack);   // 释放用户栈
      threads[i].tid = 0;
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
      threads[i].tid = 0;
      threads[i].ustack = 0;
      threads[i].used = 0;
    }
  }
  void* stack = malloc(PGSIZE);   // allocate one page for user stack
  int tid = clone(start_routine, arg, stack); // system call for kernel thread
  add_thread(&tid, stack);  // save new thread to thread table
//  printf(1, "arg addr= %x\n", (int)(stack + 4096 - 4));
//  printf(1, "PC = %x\n", *(int*)(stack + 4096 - 8));
  return tid;
}

int thread_join(void) {
  for(int i = 0; i < NTHREAD; i ++) {
    if(threads[i].used == 1) {
      int tid = join();      // 回收子线程
      if(tid > 0) {
        remove_thread(&tid);
        return tid;
      }
    }
  }
  return 0;
}

void printTCB(void) {
  for(int i = 0; i < NTHREAD; i ++)
    printf(1, "TCB %d: %d\n", i, threads[i].used);
}
