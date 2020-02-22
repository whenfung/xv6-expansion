#!/usr/bin/perl

open(SIG, $ARGV[0]) || die "open $ARGV[0]: $!"; # 打开命令行参数指出的文件

$n = sysread(SIG, $buf, 1000);  # 读入 1000 字节到 buf 中

if($n > 510){    # 如果读入字节数大于 510
  print STDERR "boot block too large: $n bytes (max 510)\n"; # 大于盘块大小
  exit 1;
}

print STDERR "boot block is $n bytes (max 510)\n"; # 提示启动扇区的有效字节数

$buf .= "\0" x (510-$n);
$buf .= "\x55\xAA";   # 启动盘块最后两个字节

open(SIG, ">$ARGV[0]") || die "open >$ARGV[0]: $!";
print SIG $buf;    # 将缓冲区内容写回文件
close SIG;
