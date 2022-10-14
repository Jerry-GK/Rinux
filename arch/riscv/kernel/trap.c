// trap.c 
#include "trap.h"
#include "types.h"
#include "clock.h"
#include "printk.h"

void exception_handler(unsigned long scause, unsigned long sepc)
{
    //ignore
}

void interrupt_handler(unsigned long scause, unsigned long sepc)
{
    switch(scause & ~TRAP_SCAUSE_MASK)
    {
        case SUPERVISOR_TIMER_INTERRUPT:
            // �����timer interrupt ���ӡ��������Ϣ, ��ͨ�� `clock_set_next_event()` ������һ��ʱ���ж�
            printk("\nRinux kernel is running!\n");
            //printk("Current CPU Time %d: ", get_cycles());
            printk("[S] Supervisor Mode Timer Interrupt\n");
            clock_set_next_event();
            break;
        default:
            break; // ignore other interruption
    }
}

void trap_handler(unsigned long scause, unsigned long sepc) {
    // ͨ�� `scause` �ж�trap����
    if(scause & TRAP_SCAUSE_MASK) //TRAP_SCAUSE_MASK = (1UL << 63)
    {
        // �����interrupt �ж��Ƿ���timer interrupt
        interrupt_handler(scause, sepc);
    }
    else
    {
        exception_handler(scause, sepc);
    }

    // ����interrupt / exception ����ֱ�Ӻ���
}
