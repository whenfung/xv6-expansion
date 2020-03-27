#include "types.h"
#include "user.h"

#define NTHREAD 64
#define PGSIZE  4096

struct {
  int pid;
  void* ustack;
  int used;
} threads[NTHREAD];   // TCB 表

// add thread to thread table
void add_thread(int* pid, void* ustack) {
  for(int i = 0; i < NTHREAD; i++) {
    if(threads[i].used == 0) {
      threads[i].pid = *pid;
      threads[i].ustack = ustack;
      threads[i].used = 1;
    }
  }
}

void remove_thread(int* pid) {
  for(int i = 0; i < NTHREAD; i ++) {
    if(threads[i].used && threads[i].pid == *pid) {
      free(threads[i].ustack);
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
    for(int i = 0; i < NTHREAD; i++) {
      threads[i].pid = 0;
      threads[i].ustack = 0;
      threads[i].used = 0;
    }
  }
  void* stack = malloc(PGSIZE);   // allocate one page for user stack
  int pid = clone(start_routine, arg, stack); // system call for kernel thread
  add_thread(&pid, stack);  // save new thread to thread table
  return pid;
}

int thread_join(void) {
  for(int i = 0; i < NTHREAD; i ++) {
    if(threads[i].used == 1) {
      int pid = join(&threads[i].ustack);
      if(pid > 0) {
        remove_thread(&pid);
        return pid;
      }
    }
  }
  return 0;
}
