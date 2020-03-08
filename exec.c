#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  begin_op();  // 调用文件系统调用时都要先调用 begin_op()

  if((ip = namei(path)) == 0){  // 查找文件的索引节点是否存在 
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);   // 对索引节点的使用都要加锁，以保证数据有效
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))  // 读取 elf 头
    goto bad;
  if(elf.magic != ELF_MAGIC)     // 验证是否为 ELF 文件
    goto bad;

  if((pgdir = setupkvm()) == 0) // 分配页表并设置内核区
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){  // 根据 elf 头信息读取 elf 文件
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))   // 读取相应的程序节头
      goto bad;
    if(ph.type != ELF_PROG_LOAD)  // 只加载标记为 LOAD 的程序段
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);  // 释放节点
  end_op();        // 完成文件系统调用
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));   // 保护页
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;        // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;   // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;     // 更新页表
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  switchuvm(curproc);
  freevm(oldpgdir);           // 释放原先的页表
  return 0;

 bad:
  if(pgdir)          // 删除页表，释放物理页帧
    freevm(pgdir);
  if(ip){
    iunlockput(ip);  // 删除索引节点和并释放刚刚申请的盘块
    end_op();
  }
  return -1;
}
