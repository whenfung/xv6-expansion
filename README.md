本节实验将修改 XV6 的调度机制，为原来的时间片轮转调度（Round Robin）增加优先级，只调度最高优先级进程，如果有多个相同最高优先级的进程，则这些进程间按照 RR 方式调度。为了实现优先级调度，我们需要完成

1. 在进程控制块中加入优先级成员，并设置默认值。
2. 提供修改优先级的系统调用。
3. 需要修改调度器的调度算法，这里的优先级是静态的，所以是抢占式静态优先级调度算法。
4. 提供测试样例。

## 1. 添加优先级属性

给 `proc` 结构体，增加成员 [priority](https://github.com/professordeng/xv6-expansion/blob/priority/proc.h#L52)。

既然有了优先级，那么在创建进程的时候就需要指定一个优先级或设置一个默认优先级。我们选择创建时使用默认优先级，后续需要的时候再调整优先级的方案。因此创建进程时分配 `proc` 结构体的 [allocproc()](https://github.com/professordeng/xv6-expansion/blob/priority/proc.c#L91) 需要设置新进程的 `priority` 成员，并将 10 作为普通进程的默认优先级。

为了使 `init` 和 `sh` 下面执行的命令可以优先运行，比如后面我们要通过 `chpr` 直接在 shell 下修改普通进程的优先级，需要在 `exec()` 切换 `curproc` 之前设置 [curproc->priority](https://github.com/professordeng/xv6-expansion/blob/priority/exec.c#L102) 为 3，也就是说 `exec()` 拷贝的进程优先级皆为 3。

为了能查看进程的优先级，我们需要修改 `proc.c` 中的 [procdump()](https://github.com/professordeng/xv6-expansion/blob/priority/proc.c#L542) 函数，使之能打印优先级信息。除了打印进程优先级 `priority` 之外，在格式上也有一点小修改， 使得打印输出的效果更好一点。 

## 2. 设置优先级

既然有基于优先级的调度，那么就需要提供设置优先级的系统调用。由于之前讨论过如何添加新的系统调用，这里只给出简要说明和核心代码。这个新的系统调用起名为 `chpr()`。简要步骤如下：

2. 在 [user.h](https://github.com/professordeng/xv6-expansion/blob/priority/user.h#L26) 中增加用户态函数原型声明，入参是进程号和新的优先级。 
3. 在 [usys.S](https://github.com/professordeng/xv6-expansion/blob/priority/usys.S#L32) 中添加 `chpr()` 函数的汇编实现代码（宏展开后对应于 `sys_chpr` 函数）。
4. 在 [syscall.h](https://github.com/professordeng/xv6-expansion/blob/priority/syscall.h#L23) 中为新的系统调用定义其编号（必须和其他系统调用编号不同）。
5. 修改 `syscall.c` 中的系统调用的函数跳转表 [syscalls[]](https://github.com/professordeng/xv6-expansion/blob/priority/syscall.c#L106)。由于 `syscall.c` 中未定义 `sys_chpr()` ，因此需要在 `syscall.c` 中增加 [sys_chpr()](https://github.com/professordeng/xv6-expansion/blob/priority/syscall.c#L106) 的函数声明。
6. 在 `sysproc.c` 中实现 [sys_chpr()](https://github.com/professordeng/xv6-expansion/blob/priority/sysproc.c#L93)。先获取 `pid` 和 `pr` 两个参数，然后调用 `chpr(pid, pr)` 将编号为 `pid` 的进程优先级设置为 `pr`。
7. 在 `proc.c` 中实现和 `proc` 相关的 [chpr()](https://github.com/professordeng/xv6-expansion/blob/priority/proc.c#L553)。
8. 最后在 `defs.h` 的 `proc.c` 部分添加 [chpr()](https://github.com/professordeng/xv6-expansion/blob/priority/defs.h#L123) 的函数原型声明，以便内核代码访问该函数。

顺便添加一个 [chpr](https://github.com/professordeng/xv6-expansion/blob/priority/chpr.c) 命令实现在 shell 下修改进程的优先级。

补充：由于获取参数时需要用到 [atoi()](https://github.com/professordeng/xv6-expansion/blob/priority/ulib.c#L84)，而该函数不支持提取负整数，所以我将其进行了扩展。

## 3. 修改调度器

为进程添加优先级的信息后，还需要在调度器中修改调度行为。我们并没有对进程控制块数组进行改动，而是增加一个 [struct proc *highP](https://github.com/professordeng/xv6-expansion/blob/priority/proc.c#L335) 变量来记录优先级最高的就绪进程。

`highP` 通过遍历 `ptable` 并指向优先级最高的就绪进程。`highP` 的初始值是理应下一个执行的进程 `p`，如果没有比它大的进程，那么就是 `p` 执行，否则 `p` 的机会将被抢占。

因此，若出现了多个最高优先级的进程，那么这些进程的调度是 RR 调度。

## 4. 验证优先级调度

首先在 `param.h` 中确认 [NCPU](https://github.com/professordeng/xv6-expansion/blob/priority/param.h#L3) 的数目为 2，也就是说系统最多可有两个进程同时运行，易于观察。

编写 [prio.c](https://github.com/professordeng/xv6-expansion/blob/priority/prio.c)，生成多个不同优先级的进程，并执行一些任务使其处于 `RUNNABLE` 或 `RUNNING` 状态，用于观察。

`prio` 命令格式：`prio nkid k`，例子如下

```bash
piro 1 2  # 生成一个子程序，并分配 2 倍的任务给子程序，运行时间自然变长
```

执行下面指令，后台会有 3 个程序在执行任务。

```bash
make qemu-nox    # 启动系统
prio 3 10&       # 创建进程并在后台执行 
```

然后利用 `chpr` 命令将其中两个状态为 `runnable` 或 `run` 的程序的优先级设置为 4。按 `Ctrl + p` 观察发现，正在运行的程序始终是优先级为 4 的两个进程。

有三个程序处于 `sleep` 状态，分别是 `init`、`sh` 和执行 `wait()` 后的父进程。还有两个优先级为 4 的进程处于 `run` 状态，一个优先级为 10 的处于 `runnable` 状态。这说明系统将 CPU 资源都分配给了高优先级的进程。

可以尝试多创建几个进程，观察进程状态。如果学有余力，可以修改内核代码支持优先级老化现象（即高优先级的进程运行久了后优先级会慢慢变低）。