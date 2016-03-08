// 此处代码主要参考os2.c，仅保留启动分页部分代码

#include <u.h>

enum { // page table entry flags
  PTE_P   = 0x001,       // 位1，表示物理页
  PTE_W   = 0x002,       // 位2，表示物理内存页内容可写
  PTE_U   = 0x004,       // 位3，表示用户态的软件可以读取对应地址的物理内存页内容
};

char pg_mem[6 * 4096]; // page dir + 4 entries + alignment

int *pg_dir, *pg0, *pg1, *pg2, *pg3;


out(port, val)  { asm(LL,8); asm(LBL,16); asm(BOUT); }
//a=参数value，然后设置页目录表起始地址为a，在调用时传入的参数为pg_dir，在setup_paging中赋值
pdir(value)     { asm(LL,8); asm(PDIR); }
spage(value)    { asm(LL,8); asm(SPAG); }
halt(value)     { asm(LL,8); asm(HALT); }

setup_paging()
{
  int i;
  //设置页表的初始地址
  pg_dir = (int *)((((int)&pg_mem) + 4095) & -4096);
  //分四页，每页1024*4个字节，即4KB
  pg0 = pg_dir + 1024;
  pg1 = pg0 + 1024;
  pg2 = pg1 + 1024;
  pg3 = pg2 + 1024;
  
  pg_dir[0] = (int)pg0 | PTE_P | PTE_W | PTE_U;  // identity map 16M
  pg_dir[1] = (int)pg1 | PTE_P | PTE_W | PTE_U;
  pg_dir[2] = (int)pg2 | PTE_P | PTE_W | PTE_U;
  pg_dir[3] = (int)pg3 | PTE_P | PTE_W | PTE_U;
  //这一步是用来对齐的吗？一个物理内存块是4KB，页表初始地址管理四个页块，而pg0是从pg_dir+1024开始的，因此把4~1024这部分赋值为0
  for (i=4;i<1024;i++) pg_dir[i] = 0;
  //初始化四个页面。每个页起初都是可用的，又因为一个4KB页为2^12，因此页块的物理地址低12位都是0，用于存储页的状态（PTE_P | PTE_W | PTE_U）。
  for (i=0;i<4096;i++) pg0[i] = (i<<12) | PTE_P | PTE_W | PTE_U;  // trick to write all 4 contiguous pages
  //页起始地址设置
  pdir(pg_dir);
  //设置页机制(使能)
  spage(1);
}

main()
{
  int t, d; 
  // reposition stack within first 16M
  //LI即将imme（立即数）赋值给a寄存器，此处a=4M
  asm(LI, 4*1024*1024); 
  //kernel sp指针指向a(4M)
  asm(SSP); 
  setup_paging();
  out(1,'1');
  halt(0);
}
