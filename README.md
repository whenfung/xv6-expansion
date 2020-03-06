实际上 XV6 只允许扩展和收缩进程空间（虚拟空间），其进程空间是连续的。学习 XV6 进程空间的内存管理方法：

1. 从 `sysproc.c` 中的 [sys_sbrk()](https://github.com/professordeng/xv6-expansion/blob/master/sysproc.c#L45) 系统调用入手，`sys_sbrk()` 将进一步调用 [growproc(n)](https://github.com/professordeng/xv6-expansion/blob/master/proc.c#L156) 进行内存空间的调整。`proc.c` 中的 `growproc(n)` 根据 n 的正负不同，分别利用 `allocuvm()` 进行扩展或 `deallocuvm()` 进行收缩。 
2. `vm.c` 中的 [allocvum()](https://github.com/professordeng/xv6-expansion/blob/master/vm.c#L219) 和 [deallocuvm()](https://github.com/professordeng/xv6-expansion/blob/master/vm.c#L251) 则是需要大家认真分析的。学习这两个函数时，要明确区分：物理页帧、页表、虚存地址三者之间的关系。其中 [kalloc()](https://github.com/professordeng/xv6-expansion/blob/master/kalloc.c#L79) 将分配一个物理页帧，[kfree()](https://github.com/professordeng/xv6-expansion/blob/master/kalloc.c#L54) 将释放一个页帧；[mappages()](https://github.com/professordeng/xv6-expansion/blob/master/vm.c#L57) 用于将建立虚存地址和物理页帧之间的页表影射。

对 XV6 的进程空间的扩展和收缩有了解之后，可以思考如何实现 Linux 方式的 `alloc()` 和 `free()`，因为它们可能造成内存空间中的孔洞，而 XV6 当前并不支持这样的内存布局。 

## 1. 数据结构

XV6 的进程空间只有一个连续区间，只允许扩展和收缩两个操作，因此只需要一个 [sz](https://github.com/professordeng/xv6-expansion/blob/vma/proc.h#L45) 成员就可以记录。如果要实现类似 Linux 操作系统的内存分配 `alloc()` 和释放 `free()` 操作，那么就可能在进程空间上造成空洞，这种不连续的空间需要其他额外信息来描述。

我们定义一个连续内存空间的描述符 [vma](https://github.com/professordeng/xv6-expansion/blob/vma/proc.h#L37) 结构体，进程控制块添加 [vm[10]](https://github.com/professordeng/xv6-expansion/blob/vma/proc.h#L58)，可以记录 9 个连续内存。

修改 `proc.c` 中的 [procdump()](https://github.com/professordeng/xv6-expansion/blob/vma/proc.c#L571)，增加内存影像的输出，因此只需要按 `Ctrl + p` 就可以将每个进程的各 `vma` 起始地址和长度显示出来。

## 2. 申请分配和删除内存操作 

添加 [myfree()](https://github.com/professordeng/xv6-expansion/blob/vma/user.h#L26) 和 [myalloc()](https://github.com/professordeng/xv6-expansion/blob/vma/user.h#L27) 系统调用，在分配空间时需要：

1. 查找合适的地址范围，并创建 `vma` 进行描述。
2. 要用 [kalloc()](https://github.com/professordeng/xv6-expansion/blob/vma/kalloc.c#L79) 分配足够的物理页帧，并用 [mappages()](https://github.com/professordeng/xv6-expansion/blob/vma/vm.c#L57) 将这些页帧影射到所指定的虚存地址上。

在释放空间时需要：

1. 将所涉及的页帧，逐个解除页表影射。
2. 删除 `vma`。我们这里分配和释放内存空间，都以页（4 KB）的整数倍为大小，以减少编成细节、减轻大家的编程工作量。

在我的实现中，[mygrowproc()](https://github.com/professordeng/xv6-expansion/blob/vma/proc.c#L181) 完成虚拟空间的检测和 `vma` 的分配，而 [myreduceproc()](https://github.com/professordeng/xv6-expansion/blob/vma/proc.c#L211) 则完成虚拟空间的回收。

[myallocuvm()](https://github.com/professordeng/xv6-expansion/blob/vma/vm.c#L251) 完成虚拟空间到物理页帧的映射，而 [mydeallocuvm()](https://github.com/professordeng/xv6-expansion/blob/vma/vm.c#L301) 完成虚拟空间到物理页帧的解绑。

### 2.1 思考

如果允许用户分配任意尺寸的空间，例如分配了 16 个字节和 32 字节的两个空间，那么你使用两个页帧来映射它们，还是用一个页帧来映射它们？如果用一个页来影射着 48 个字节，那么释放时又如何处理，你能提供一个比较完整的解决方案使得页帧的空间利用率较高？

**参考答案**：可以在内核态实现，也可以将这些细节封装成用户态库，用户直接调用函数即可。推荐使用用户态库，尽量保证内核态的简洁和高效。

## 3. 辅助代码

为了测试新的内核代码，必须添加相应的系统调用 `myalloc()` 和 `myfree()`，这里直接给出相应的辅助代码：

1. 在 [user.h](https://github.com/professordeng/xv6-expansion/blob/vma/user.h#L26) 添加函数声明。
2. 在 [usys.S](https://github.com/professordeng/xv6-expansion/blob/vma/usys.S#L32) 中添加系统调用入口。
3. 在 [syscall.h](https://github.com/professordeng/xv6-expansion/blob/vma/syscall.h#L23) 中定义系统调用编号。
4. 修改 `syscall.c` 中的系统调用跳转表 [syscalls[]](https://github.com/professordeng/xv6-expansion/blob/vma/syscall.c#L131)，并添加外部函数声明 [sys_myalloc()](https://github.com/professordeng/xv6-expansion/blob/vma/syscall.c#L106) 和 `sys_myfree()`。
5. 在 [sysproc.c](https://github.com/professordeng/xv6-expansion/blob/vma/sysproc.c#L93) 实现系统调用函数。
6. 在 [defs.h](https://github.com/professordeng/xv6-expansion/blob/vma/defs.h#L110) 在添加新的内核函数声明，以便其他文件的内核代码调用。 

## 4. 测试代码

编写应用程序 [myalloc.c](https://github.com/professordeng/xv6-expansion/blob/vma/myalloc.c) 并添加到 XV6，该程序连续分配 5 个空间，然后释放其中的 2、4。

1. 察看内存空间是否有空洞。 
2. 发出指向内存空洞的区间，是否会引发进程非法操作而撤销。

开启系统后，运行 `myalloc`，然后该程序会睡眠一段时间，再按 `Ctrl + p` 可以看到具体的空间。如下：

```bash
pid: 3, state: sleep , name: myalloc
start: 12288, length: 8192
start: 32768, length: 4096
start: 65536, length: 36864
```

若试图在空洞里面写数据，会出现如下结果：

```bash
pid 3 myalloc: trap 14 err 6 on cpu 1 eip 0x82 addr 0x5000--kill proc
```





