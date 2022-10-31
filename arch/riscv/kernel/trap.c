// trap.c 
#include "trap.h"
#include "types.h"
#include "clock.h"
#include "printk.h"
#include "proc.h"

int exit = 0;

void exception_handler(unsigned long scause, unsigned long sepc)
{
    //ignore
    if(exit==0)
        printk("[S] Exeception happens!!!!!!!!!!!!!!!!!!!!!\n");
    exit = 1;
}

void interrupt_handler(unsigned long scause, unsigned long sepc)
{
    switch(scause & ~TRAP_SCAUSE_MASK)
    {
        case SUPERVISOR_TIMER_INTERRUPT:
            // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟中断
            //printk("\nRinux kernel is running!\n");
            //printk("Current CPU Time %d: ", get_cycles());
            printk("\n[S] Supervisor Mode Timer Interrupt\n");
            clock_set_next_event();
            //调度控制函数
            do_timer();
            break;
        default:
            printk("[S] Supervisor Mode Not-timer Interrupt\n");
            break; // ignore other interruption
    }
}

void trap_handler(unsigned long scause, unsigned long sepc) {
    // 通过 `scause` 判断trap类型
    if(scause & TRAP_SCAUSE_MASK) //TRAP_SCAUSE_MASK = (1UL << 63)
    {
        // 如果是interrupt 判断是否是timer interrupt
        interrupt_handler(scause, sepc);
    }
    else
    {
        exception_handler(scause, sepc);
    }

    // 其他interrupt / exception 可以直接忽略
}
