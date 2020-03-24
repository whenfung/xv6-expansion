	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register; it is caller saved.
 */
	.globl thread_switch
thread_switch:
	/* YOUR CODE HERE */
  // C 语言函数调用会压入下一条语句的 eip
  pushal                        // 8 个 x86 通用寄存器
  // eax 指向 sp
  movl current_thread, %eax  
  // save sp
  movl %esp, (%eax)            // 将 esp 保存到线程栈中


  movl next_thread, %eax       // 下一个线程的结构体
  movl %eax, current_thread    // current_thread = next_thread
  // restore sp
  movl (%eax), %esp            // 恢复栈顶指针

  popal                        // 弹出 8 个 x86 通用寄存器
  
  movl $0x0, next_thread       // next_thread = 0

  // popal 后 esp 指向 return address	
  ret				/* pop return address from stack */
