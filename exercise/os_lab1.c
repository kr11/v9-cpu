// 参考os4.c以及ucore_lab_1_6的练习题
//本题的实现效果为：每个一段时间在屏幕上输出一个tasks。
#include <u.h>

//在中断时可能的各种状态，在第一题中我们重点关注的是FTIMER，即时钟中断并作出反应。
enum {    // processor fault codes
  FMEM,   // bad physical address
  FTIMER, // timer interrupt
  FKEYBD, // keyboard interrupt
  FPRIV,  // privileged instruction
  FINST,  // illegal instruction
  FSYS,   // software trap
  FARITH, // arithmetic trap
  FIPAGE, // page fault on opcode fetch
  FWPAGE, // page fault on write
  FRPAGE, // page fault on read
  USER=16 // user mode exception 
};

char task0_stack[1000];
char task0_kstack[1000];

int *task0_sp;

//LL为设置寄存器a，有许多指令都是直接看a的状态的，例如下面四个命令函数；
//BOUT:a = write(a, &b, 1);此命令是向标准输出端输出字符
out(port, val)  { asm(LL,8); asm(LBL,16); asm(BOUT); }
//IVEC:设置中断向量起始地址,该函数根据isr所指的值设置中断向量起始地址。
ivec(void *isr) { asm(LL,8); asm(IVEC); }
//TIME:设置timer的timeout，到达timeout则调用中断，从而起到了定是执行的功能；
stmr(int val)   { asm(LL,8); asm(TIME); }
//停止
halt(value)     { asm(LL,8); asm(HALT); }
//用到了out函数；字符串起点为*p，输出n个字符
sys_write(fd, char *p, n) { int i; for (i=0; i<n; i++) out(fd, p[i]); return i; }

task0()
{
  while(1);
}

trap(int *sp, int c, int b, int a, int fc, unsigned *pc)
//trap(int *sp, int c, int b, int a, int fc, unsigned *pc)
{
  switch (fc) {
  case FTIMER + USER:
    if (ticks % 20000 == 0) {
        sys_write(1, "ticks\n", 6);  
    }
    break;  
  default:
    default: sys_write(1, "panic! unknown interrupt\n", 25); asm(HALT);  
  }
}

//中断时保存所有寄存器数据，然后切换到内存态，执行trap()函数，然后恢复各寄存器值，中断返回（RTI）
alltraps()
{
  asm(PSHA);
  asm(PSHB);
  asm(PSHC);
  asm(LUSP);
  asm(PSHA);
  trap();                // registers passed by reference/magic
  asm(POPA);
  asm(SUSP);
  asm(POPC);
  asm(POPB);
  asm(POPA);
  asm(RTI);
}

trapret()
{
  asm(POPA); //a=*sp, sp+=8 byte
  asm(SUSP); //usp=a
  asm(POPC); //c=*sp, sp+=8 byte
  asm(POPB); //b=*sp, sp+=8 byte
  asm(POPA); //a=*sp, sp+=8 byte
  asm(RTI);  //return from interrupt, mode=USER, pc=&task0
}

main()
{
  int *kstack;
  
  //设置timeout的时间，用以引发中断
  stmr(1000);
  //中断调用函数
  ivec(alltraps);
  task0_sp = &task0_kstack[1000];
  task0_sp -= 2; *task0_sp = &task0;
  task0_sp -= 2; *task0_sp = USER; // fault code
  task0_sp -= 2; *task0_sp = 0; // a
  task0_sp -= 2; *task0_sp = 0; // b
  task0_sp -= 2; *task0_sp = 0; // c
  task0_sp -= 2; *task0_sp = &task0_stack[1000]; //user stack
  task0_sp -= 2; *task0_sp = &trapret;
  kstack = task0_sp; //kernel stack
  
  asm(LL, 4); // a = kstack
  asm(SSP);   // sp = a
  asm(LEV);   // return
}