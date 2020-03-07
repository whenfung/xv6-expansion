虽然 XV6 提供了自旋锁用于内核代码的并发同步，但是用户态并没有提供同步手段。我们这里尝试实现一个简单的信号量机制来为用户进程同步，内部实现上仍是建立在内核自旋锁之上，加上了阻塞睡眠的能力从而避免 “忙等” 的问题。 

由于 XV6 没有提供类似共享内存这样的共享资源，我们就在系统中定义一个共享整数变量 `sh_var`，通过 `sh_var_read()` 和 `sh_var_write()` 进行读写操作。以此作为验证信号量工作正常的功能展示。

## 1. 共享变量及其访问

验证信号量的时候需要提供临界资源，因此我们在 `spinlock.c` 定义了 [sh_var](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L15) 全局变量。为了让其他内核代码能访问该变量，还需要在 `defs.h` 中添加 [sh_var](https://github.com/professordeng/xv6-expansion/blob/sem/defs.h#L137) 的变量声明。

## 1.1 访问共享变量

为了访问这个共享变量，需要提供 `sh_var_read()` 和 `sh_var_write()` 两个系统调用来完成读写操作。添加系统调用的方法在之前讨论过，这里就简单给出添加过程：

1. 在 [user.h](https://github.com/professordeng/xv6-expansion/blob/sem/user.h) 中声明两个用户态函数原型，供进程调用。
2. 在 [usys.S](https://github.com/professordeng/xv6-expansion/blob/sem/usys.S#L32) 中提供系统调用入口。
3. 在 [syscall.h](https://github.com/professordeng/xv6-expansion/blob/sem/syscall.h#L23) 定义读写操作的系统调用编号，作为系统调用表的索引。
4. 修改 `syscall.c` 中的系统调用跳转表 [syscalls[]](https://github.com/professordeng/xv6-expansion/blob/sem/syscall.c#L135)。在 `syscall.c` 中提供两个外部函数声明（[syscall.c#L106](https://github.com/professordeng/xv6-expansion/blob/sem/syscall.c#L106)）。
5. 在 [sysproc.c](https://github.com/professordeng/xv6-expansion/blob/sem/sysproc.c#L93) 中实现这两个系统调用。

### 1.2 无互斥的并发访问

定义了共享变量以及访问的系统调用之后，我们可以在应用程序中尝试并发访问 `sh_var`。编写 [sh_rw_nolock.c](https://github.com/professordeng/xv6-expansion/blob/sem/sh_rw_nolock.c)。同时还要需要修改 `Makefile` 的 `$UPROGS`，添加一个 `_sh_rw_nolock\`。

启动 XV6 后，执行 `sh_rw_nolock`，不出意外的话结果应该小于 `200000`，即生成的两个子程序并发访问 `sh_var` 的时候发生了数据踩踏。

## 2. 信号量数据结构

为了实现信号量，除了创建、撤销、P 操作、V 操作外，还需要添加新的数据结构、初始化函数、调整 `wakeup` 唤醒操作等。

为了管理信号量我们定义了 [sem](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.h#L12) 结构体，其中 `resources` 成员用于记录信号量中资源的数量， `lock` 内核自旋锁是为了让信号量的操作保持原子性，`allocated` 用于表示该信号量是否已经被分配使用，`used` 用来统计当前所需（所用）资源。

整个系统内部只有一个信号量数组 [sems[128]](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L13)，也就是说用户进程申请的信号量总数不超过 128 个。

## 3. 信号量操作的系统调用

为了实现信号量，我们需要增加四个系统调用，分别是：

1. 创建信号量 `sem_create()`，其参数是信号量的初值（例如互斥量则用 1 做初值），返回值是信号量的编号，即内核变量 `sems[]` 数组的下标。
2. `sem_p()` 则是对指定编号的信号量进行 P 操作（减一操作、down 操作），P 操作后若信号量小于 0，进程进入该事件的阻塞队列。
3. `sem_v()` 则是对指定 `id` 的信号量进行 V 操作（增一操作、up 操作），V 操作后若信号量大于 0，则唤醒该事件中的阻塞队列中的其中一个进程。
4. `sem_free()` 释放指定 `id` 的信号量。

### 3.1 核心代码

我们将信号量的核心实现代码放在 `spinlock.c` 中，而不是用独立的 C 文件，从而避免增加 `Makefile` 上的修改工作。 

[initsem()](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L16)

系统启动时要调用 `initsem()` 对信号量进行初始化。`initsem()` 完成的工作很简单，就是完成信号量数组的自旋锁的初始化。

然后我们在 `main.c` 的 `main()` 中插入 [initsem()](https://github.com/professordeng/xv6-expansion/blob/sem/main.c#L36)。为了让 `main.c` 能调用 `initsem()`，还需要在 `defs.h` 声明 [initsem()](https://github.com/professordeng/xv6-expansion/blob/sem/defs.h#L136) 函数原型。 

[sys_sem_create()](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L24)

`sys_sem_create()` 扫描 `sems[]` 数组，查看里面 `allocated` 标志，发现未用的信号量则将其 `allocated` 置 1，即可返回其编号。如果扫描一次后未发现，则返回 -1。注意每次操作时需要对 `sems[i]` 进行加锁操作，检查完成后进行解锁。

[sys_sem_free()](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L43)

`sys_sem_free()` 将指定 `i` 作为下标访问 `sems[i]` 获得当前信号量 `sems[i]`，然后对 `sems[i].lock` 加锁，判定该信号量上没有睡眠阻塞的进程，若无其他进程使用则将 `sems[i].allocated` 标志设置为未使用，从而释放信号量，最后对 `sems[i].lock` 解锁。

[sys_sem_p()](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L60)

`sys_sem_p()` 将指定 `i` 作为下标访问 `sems[i]` 获得当前信号量 `sem`，然后用 `acquire()` 对 `sems[i].lock` 加锁，加锁成功后 `sems[i].resources--`，如果发现 `sems[i].resources < 0` 则睡眠,，否则用 `realease()` 解锁退出临界区并直接返回。 

注意：`sleep()` 会将 [sems[i].lock](https://github.com/professordeng/xv6-expansion/blob/sem/proc.c#L434) 吐出来。

[sys_sem_v()](https://github.com/professordeng/xv6-expansion/blob/sem/spinlock.c#L68)

`sys_sem_v()` 将指定 `i` 作为下标访问 `sems[i]` 获得当前信号量 `sem`，然后对 `sem.lock` 加锁， 加锁成功后 `sem.resources++`，如果发现 `sem.resources < 1`，说明有阻塞进程在等待资源，唤醒该信号量上阻塞的睡眠进程，然后解锁 `sem.lock` ，返回 0 表示操作成功。

### 3.2 修改 wakeup 操作 

由于 XV6 系统自带的 `wakup` 操作会将所有等待相同事件的进程唤醒，因此需要重写一个新的 `wakeup` 操作函数 [wakup1p()](https://github.com/professordeng/xv6-expansion/blob/sem/proc.c#L536)，仅唤醒等待指定信号量的一个进程，从而避免 “群惊” 效应。另外，还需要在 [defs.h](https://github.com/professordeng/xv6-expansion/blob/sem/defs.h#L122) 中声明该函数原型。

### 3.3 系统调用的辅助代码

除了上述四个系统调用的核心实现代码外，还有系统调用号的设定、用户入口函数、系统调用跳转表的修改等工作，一并在此给出，以便读者操作时对照检查。

1. 在 [syscall.h](https://github.com/professordeng/xv6-expansion/blob/sem/syscall.h#L25) 为新添的四个系统调用进行编号。
2. 在 [user.h](https://github.com/professordeng/xv6-expansion/blob/sem/user.h#L28) 中声明四个用户态函数原型。
3. 在 [usys.S](https://github.com/professordeng/xv6-expansion/blob/sem/usys.S#L34) 中为四个系统调用添加函数入口。
4. 修改 `syscall.c` 中的系统调用跳转表 [syscalls[]](https://github.com/professordeng/xv6-expansion/blob/sem/syscall.c#L137)。并在 [syscall.c](https://github.com/professordeng/xv6-expansion/blob/sem/syscall.c#L108) 的 `syscalls[]` 数组前面声明上述函数是外部函数。

## 4. 测试代码

我们重新编写一个访问共享变量的应用程序 [sh_rw_lock.c](https://github.com/professordeng/xv6-expansion/blob/sem/sh_rw_lock.c)，并且加上信号量的互斥控制。修改 `Makefile` 为 `UPROGS` 添加一个 `_sh_rw_lock\`，重新编译生成系统。

执行 `sh_rw_lock` 后观察结果，其中有一个进程的结果是 20000，说明互斥锁有效。