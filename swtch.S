# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl swtch
swtch:
  movl 4(%esp), %eax
  movl 8(%esp), %edx

  # Save old callee-saved registers
  pushl %ebp     # 压栈
  pushl %ebx     # 压栈
  pushl %esi     # 压栈
  pushl %edi     # 压栈

  # Switch stacks
  movl %esp, (%eax)
  movl %edx, %esp

  # Load new callee-saved registers
  popl %edi    # 弹栈
  popl %esi    # 弹栈
  popl %ebx    # 弹栈
  popl %ebp    # 弹栈
  ret
