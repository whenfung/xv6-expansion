#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

// Simple logging that allows concurrent FS system calls.
//
// A log transaction contains the updates of multiple FS system
// calls. The logging system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark
// its start and end. Usually begin_op() just increments
// the count of in-progress FS system calls and returns.
// But if it thinks the log is close to running out, it
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks.
// The on-disk log format:
//   header block, containing block #s for block A, B, C, ...
//   block A
//   block B
//   block C
//   ...
// Log appends are synchronous.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memory of logged block# before commit.
struct logheader {
  int n;                  // 计数，0 表示没有修改的缓存块
  int block[LOGSIZE];     // 记录数据号，用在 install_trans. LOGSIZE = 30，定义在 param.h
};

struct log {
  struct spinlock lock;   // 访问日志需要互斥 
  int start;              // 日志起始盘块
  int size;               // 日志大小
  int outstanding; // how many FS sys calls are executing.
  int committing;  // in commit(), please wait.
  int dev;                // 日志所处的设备号
  struct logheader lh;
};
struct log log;

static void recover_from_log(void);
static void commit();

void
initlog(int dev)    // 在 forkret 被初始化，只调用一次
{
  if (sizeof(struct logheader) >= BSIZE)  // 日志头的大小控制在 512 字节内
    panic("initlog: too big logheader");

  struct superblock sb;
  initlock(&log.lock, "log");
  readsb(dev, &sb);             // 从超级块中读取日志描述信息
  log.start = sb.logstart;      // 起始盘块号
  log.size = sb.nlog;           // 盘块数目
  log.dev = dev;                // 处于哪个设备
  recover_from_log();           // 电脑启动的操作，电脑崩溃可从此恢复
}

// Copy committed blocks from log to their home location
static void
install_trans(void)    // 将日志区数据拷贝到数据区
{
  int tail;  // tail+1 的日志块对应 block[tail] 的数据块

  for (tail = 0; tail < log.lh.n; tail++) {
    struct buf *lbuf = bread(log.dev, log.start+tail+1);   // read log block
    struct buf *dbuf = bread(log.dev, log.lh.block[tail]); // read dst
    memmove(dbuf->data, lbuf->data, BSIZE);                // copy block to dst
    bwrite(dbuf);                                          // write dst to disk
    brelse(lbuf);
    brelse(dbuf);
  }
}

// Read the log header from disk into the in-memory log header
static void
read_head(void)  // 将磁盘日志头读到内存来
{
  struct buf *buf = bread(log.dev, log.start);             // dev 和 start 由超级块提供
  struct logheader *lh = (struct logheader *) (buf->data); // 强制转换
  int i;
  log.lh.n = lh->n;
  for (i = 0; i < log.lh.n; i++) {                         // 逐块记录
    log.lh.block[i] = lh->block[i];
  }
  brelse(buf);
}

// Write in-memory log header to disk.
// This is the true point at which the
// current transaction commits.
static void
write_head(void)   // 将内存日志头写回磁盘
{
  struct buf *buf = bread(log.dev, log.start);
  struct logheader *hb = (struct logheader *) (buf->data);
  int i;
  hb->n = log.lh.n;
  for (i = 0; i < log.lh.n; i++) {
    hb->block[i] = log.lh.block[i];
  }
  bwrite(buf);
  brelse(buf);
}

static void
recover_from_log(void) // 主要用在电脑崩溃后重启
{
  read_head();
  install_trans();     // if committed, copy from log to disk
  log.lh.n = 0;
  write_head();        // clear the log
}

// called at the start of each FS system call.
void
begin_op(void)   // 只是对 outstanding 做自增操作
{
  acquire(&log.lock);
  while(1){
    if(log.committing){   // 日志正在执行提交任务
      sleep(&log, &log.lock);
    } else if(log.lh.n + (log.outstanding+1)*MAXOPBLOCKS > LOGSIZE){
      // this op might exhaust log space; wait for commit.
      sleep(&log, &log.lock);
    } else {
      log.outstanding += 1;
      release(&log.lock);
      break;
    }
  }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void
end_op(void)
{
  int do_commit = 0;

  acquire(&log.lock);
  log.outstanding -= 1;
  if(log.committing)
    panic("log.committing");
  if(log.outstanding == 0){  // 没有进程使用文件系统
    do_commit = 1;           // 执行 commit()
    log.committing = 1;      // 告诉进程正执行 commit()
  } else {
    // begin_op() may be waiting for log space,
    // and decrementing log.outstanding has decreased
    // the amount of reserved space.
    wakeup(&log);        // 唤醒等待日志资源的进程
  }
  release(&log.lock);

  if(do_commit){
    // call commit w/o holding locks, since not allowed
    // to sleep with locks.
    commit();                  // 执行提交
    acquire(&log.lock);        
    log.committing = 0;        // 提交完成
    wakeup(&log);              // 唤醒等待日志资源的进程
    release(&log.lock);
  }
}

// Copy modified blocks from cache to log.
static void
write_log(void)  // 将脏的缓存块写到日志区
{
  int tail;

  for (tail = 0; tail < log.lh.n; tail++) {
    struct buf *to = bread(log.dev, log.start+tail+1);     // 日志块
    struct buf *from = bread(log.dev, log.lh.block[tail]); // 缓存块
    memmove(to->data, from->data, BSIZE);
    bwrite(to);                                    // 执行写日志操作
    brelse(from);
    brelse(to);
  }
}

static void
commit()  // 真正的写数据区操作
{
  if (log.lh.n > 0) { // 计数器统计有多少个写数据盘操作
    write_log();      // 将修改过的缓存块数据写到日志区
    write_head();     // 将日志头写到日志区 (标志写日志操作真正完成), 如果崩溃上一步数据丢失
    install_trans();  // 将日志区数据写回数据区
    log.lh.n = 0;     // 标志完成更新文件系统
    write_head();     // 更新日志头, 如果这一步失败, 那么下次启动会重新执行 install_trans
  }
}

// Caller has modified b->data and is done with the buffer.
// Record the block number and pin in the cache with B_DIRTY.
// commit()/write_log() will do the disk write.
//
// log_write() replaces bwrite(); a typical use is:
//   bp = bread(...)
//   modify bp->data[]
//   log_write(bp)
//   brelse(bp)
void
log_write(struct buf *b)
{
  int i;

  if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)  
    panic("too big a transaction");
  if (log.outstanding < 1)   // 没有文件系统相关调用，就一定不会执行这个函数
    panic("log_write outside of trans");

  acquire(&log.lock);
  for (i = 0; i < log.lh.n; i++) {
    if (log.lh.block[i] == b->blockno)   // 检查是否已存在对应盘块
      break;
  }
  log.lh.block[i] = b->blockno;     // 标记新分配的盘块号
  if (i == log.lh.n)
    log.lh.n++;
  b->flags |= B_DIRTY;   // prevent eviction
  release(&log.lock);
}

