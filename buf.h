struct buf {
  int flags;              // 标志位
  uint dev;               // 所在设备
  uint blockno;           // 盘块号
  struct sleeplock lock;  // 使用缓存块需要加锁
  uint refcnt;            // 被进程引用的次数
  struct buf *prev;       // LRU cache list
  struct buf *next;
  struct buf *qnext;      // disk queue
  uchar data[BSIZE];      // 磁盘数据在内存中的拷贝
};
#define B_VALID 0x2       // buffer has been read from disk
#define B_DIRTY 0x4       // buffer needs to be written to disk

