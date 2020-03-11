struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type; // 文件类型
  int ref; // reference count
  char readable;         // 读权限
  char writable;         // 写权限
  struct pipe *pipe;     // 管道地址
  struct inode *ip;      // 索引节点地址
  uint off;              // 文件偏移
};


// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
