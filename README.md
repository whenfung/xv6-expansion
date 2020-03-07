调度方面的我们安排两个实验：一个是热身性质的，只是将时间片延长到多个时钟中断，所需修改的代码比较少，也不涉及添加系统调用；另一个是优先级调度实验，由于涉及系统调用以及用于验证的用户可执行程序，因此略微复杂一点。 调度对操作系统而言很关键，但是代码量并不大。相对于内存管理、文件系统系统而言， 调度代码是相对简单的。

本节只讲时间片长度调整。我们来尝试将一个进程运行的时间片扩展为 N 个时钟周期。具体思路也很简单，为每个进程的 PCB 中添加时钟计数值，调度时当前进程的时间片未用完则不切换。

## 1. 增加时间片信息

修改 [proc](https://github.com/professordeng/xv6-expansion/blob/slot/proc.h#L37) 结构体的定义，增加成员 [slot](https://github.com/professordeng/xv6-expansion/blob/slot/proc.h#L52) 并定义时间片长度为 [SLOT](https://github.com/professordeng/xv6-expansion/blob/slot/proc.h#L54) 个 `tick`。 

然后在创建进程时分配 `proc` 结构体的 [allocproc()](https://github.com/professordeng/xv6-expansion/blob/slot/proc.c#L91) 函数中设置新进程的 `slot` 成员。

为了能查看到进程时间片信息，还需要在 `proc.c` 中的 [procdump()](https://github.com/professordeng/xv6-expansion/blob/slot/proc.c#L527) 函数中将输出信息增加一项时间片剩余量。

## 2. 时间片控制

XV6 原来的是实现在每次时钟中断时就调用 `yield()` 让出 CPU 并引发一次调度。现在修改后的代码需要对时间片剩余量进行递减，以及判定当前进程时间片是否用完，决定是否需要进行调度。修改后的代码在 [trap()](https://github.com/professordeng/xv6-expansion/blob/slot/trap.c#L107) 函数中完成上述检查。

## 3. 查看时间片信息

我们编写了 [loop.c](https://github.com/professordeng/xv6-expansion/blob/slot/loop.c) 程序，用于查看进程时间片信息。`loop.c` 调用两次 `fork()`，然后进入死循环。编译前，不要忘记在 `Makefile` 的 `UPROGS` 目标中增加一 项 `_loop\`。 

在 `loop` 运行时，用 `Ctrl + p` 检查当前进程剩余的时间片。可以看出各个进程时间片所剩的 `tick` 计数。 

 ```bash
slice left: 6 ticks, 4 run    loop
slice left: 8 ticks, 5 runble loop
slice left: 8 ticks, 6 runble loop
slice left: 5 ticks, 7 run    loop

slice left: 2 ticks, 4 run    loop
slice left: 8 ticks, 5 run    loop
slice left: 8 ticks, 6 runble loop
slice left: 8 ticks, 7 runble loop

slice left: 8 ticks, 4 runble loop
slice left: 1 ticks, 5 run    loop
slice left: 1 ticks, 6 run    loop
slice left: 8 ticks, 7 runble loop
 ```

可以看到处于 runnable 的进程都有 8 个时间片，而处于 run 状态的进程则时间片不定。