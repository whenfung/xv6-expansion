// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  uint magic;     // must equal ELF_MAGIC
  uchar elf[12];
  ushort type;
  ushort machine;
  uint version;
  uint entry;
  uint phoff;     // 程序段表的偏移地址
  uint shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;   // 段表的项数
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

// Program section header
struct proghdr {
  uint type;     // 类型
  uint off;      // 磁盘偏移
  uint vaddr;
  uint paddr;
  uint filesz;   // 文件大小
  uint memsz;    // 所需内存大小
  uint flags;
  uint align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
