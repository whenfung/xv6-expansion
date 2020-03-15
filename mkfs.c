#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#include "param.h"

#ifndef static_assert
#define static_assert(a, b) do { switch (0) case 0: case (a): ; } while (0)
#endif

#define NINODES 200

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE/(BSIZE*8) + 1;    // 位图需要的盘块数
int ninodeblocks = NINODES / IPB + 1;  // 索引节点需要的盘块数
int nlog = LOGSIZE;                    // 日志需要的盘块数, 默认 30
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

int fsfd;
struct superblock sb;      // 超级块
char zeroes[BSIZE];        // 盘块大小, 内容全 0 的数组
uint freeinode = 1;        // 空闲索引节点从 1 开始
uint freeblock;


void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

// convert to intel byte order
ushort
xshort(ushort x)  // 涉及到大端和小端的知识
{
  ushort y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

uint
xint(uint x)  // 涉及到大端和小端的知识
{
  uint y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

int
main(int argc, char *argv[])
{
  int i, cc, fd;
  uint rootino, inum, off;
  struct dirent de;
  char buf[BSIZE];
  struct dinode din;


  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BSIZE % sizeof(struct dinode)) == 0); // assert 为真，继续运行
  assert((BSIZE % sizeof(struct dirent)) == 0);

  fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
  if(fsfd < 0){
    perror(argv[1]);
    exit(1);
  }

  // 1 fs block = 1 disk sector
  nmeta = 2 + nlog + ninodeblocks + nbitmap; // 元数据区
  nblocks = FSSIZE - nmeta;                  // 数据区

  sb.size = xint(FSSIZE);          // 文件系统大小
  sb.nblocks = xint(nblocks);      // 数据盘块个数
  sb.ninodes = xint(NINODES);      // 索引节点个数
  sb.nlog = xint(nlog);            // 日志所需盘块个数
  sb.logstart = xint(2);           // 日志区起点
  sb.inodestart = xint(2+nlog);    // 索引节点区起点
  sb.bmapstart = xint(2+nlog+ninodeblocks); // 位图起点

  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

  freeblock = nmeta;     // the first free block that we can allocate

  for(i = 0; i < FSSIZE; i++)   // 所有盘块清零
    wsect(i, zeroes);

  memset(buf, 0, sizeof(buf));   // 设置超级块
  memmove(buf, &sb, sizeof(sb));
  wsect(1, buf);

  rootino = ialloc(T_DIR);      // T_DIR 为 1
  assert(rootino == ROOTINO);   // 根目录的索引节点为 ROOTINO=1

  bzero(&de, sizeof(de));       // 目录结构体清零
  de.inum = xshort(rootino);    // 目录对应的索引节点
  strcpy(de.name, ".");         // 根目录初始化时只有自己 .
  iappend(rootino, &de, sizeof(de)); // 添加根目录文件 

  bzero(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, "..");
  iappend(rootino, &de, sizeof(de));

  for(i = 2; i < argc; i++){
    assert(index(argv[i], '/') == 0);

    if((fd = open(argv[i], 0)) < 0){
      perror(argv[i]);
      exit(1);
    }

    // Skip leading _ in name when writing to file system.
    // The binaries are named _rm, _cat, etc. to keep the
    // build operating system from trying to execute them
    // in place of system binaries like rm and cat.
    if(argv[i][0] == '_')
      ++argv[i];

    inum = ialloc(T_FILE);

    bzero(&de, sizeof(de));
    de.inum = xshort(inum);
    strncpy(de.name, argv[i], DIRSIZ);
    iappend(rootino, &de, sizeof(de));

    while((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);

    close(fd);
  }

  // fix size of root inode dir
  rinode(rootino, &din);
  off = xint(din.size);
  off = ((off/BSIZE) + 1) * BSIZE;
  din.size = xint(off);
  winode(rootino, &din);

  balloc(freeblock);

  exit(0);
}

void
wsect(uint sec, void *buf) // 将 buf 的数据写入 sec
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(write(fsfd, buf, BSIZE) != BSIZE){
    perror("write");
    exit(1);
  }
}

void
winode(uint inum, struct dinode *ip) // 根据 inum 将 ip 写入磁盘
{
  char buf[BSIZE];        // 盘块大小的缓存块
  uint bn;                // 盘块号
  struct dinode *dip;     // 磁盘索引节点指针

  bn = IBLOCK(inum, sb);  // 索引节点 inum 所在的盘块号
  rsect(bn, buf);         // 将第 bn 个盘块的数据读到 buf 中
  dip = ((struct dinode*)buf) + (inum % IPB); // inum 在 buf 中的偏移 
  *dip = *ip;             // 更新磁盘索引节点
  wsect(bn, buf);         // 将缓存块 buf 写到第 bn 个盘块中
}

void
rinode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *ip = *dip;
}

void
rsect(uint sec, void *buf)  // 读盘块 sec 到 buf 中
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(read(fsfd, buf, BSIZE) != BSIZE){
    perror("read");
    exit(1);
  }
}

uint
ialloc(ushort type)
{
  uint inum = freeinode++;   // freeinode 初始化为 1
  struct dinode din;         // 磁盘索引节点

  bzero(&din, sizeof(din));  // 清零
  din.type = xshort(type);   // 索引节点类型
  din.nlink = xshort(1);     // 文件硬链接数
  din.size = xint(0);        // 文件大小, 一开始为 0
  winode(inum, &din);        // 根据 inum 将 din 写到磁盘对应的位置
  return inum;
}

void
balloc(int used)
{
  uchar buf[BSIZE];
  int i;

  printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BSIZE*8);
  bzero(buf, BSIZE);
  for(i = 0; i < used; i++){
    buf[i/8] = buf[i/8] | (0x1 << (i%8));
  }
  printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  wsect(sb.bmapstart, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void
iappend(uint inum, void *xp, int n) // 给 inum 代表的文件添加数据
{
  char *p = (char*)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din);    // 根据 inum 读取磁盘索引节点信息
  off = xint(din.size);  // 读取文件大小, 数据将在后面添加
  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  while(n > 0){              // 数据未添加完
    fbn = off / BSIZE;       // 找到文件的最后一个盘块
    assert(fbn < MAXFILE);   // 文件大小不可超过 MAXFILE
    if(fbn < NDIRECT){       // 直接索引
      if(xint(din.addrs[fbn]) == 0){ // 未有映射
        din.addrs[fbn] = xint(freeblock++); // 添加映射
      }
      x = xint(din.addrs[fbn]);     // 记录映射盘块号
    } else {                        // 间接索引 
      if(xint(din.addrs[NDIRECT]) == 0){  // 未有映射
        din.addrs[NDIRECT] = xint(freeblock++);  // 添加映射
      }
      rsect(xint(din.addrs[NDIRECT]), (char*)indirect); // 读取索引盘块
      if(indirect[fbn - NDIRECT] == 0){                 // 未有映射
        indirect[fbn - NDIRECT] = xint(freeblock++);    // 添加映射
        wsect(xint(din.addrs[NDIRECT]), (char*)indirect); // 更新索引盘块
      }
      x = xint(indirect[fbn-NDIRECT]); // 记录新增的盘块号
    }
    n1 = min(n, (fbn + 1) * BSIZE - off);  // 计算已写数据大小
    rsect(x, buf);       // 读取 x 对应的盘块
    bcopy(p, buf + off - (fbn * BSIZE), n1); // 填充 buf
    wsect(x, buf);   // 更新 x 对应的盘块
    n -= n1;         // 为下一轮循环做准备
    off += n1;  
    p += n1;
  }
  din.size = xint(off);  // 更新文件大小
  winode(inum, &din);    // 更新对应盘块
}
