#ifndef _TRAP_H
#define _TRAP_H

#define TRAP_SCAUSE_MASK (1UL << 63)
#define SUPERVISOR_TIMER_INTERRUPT 5

void exception_handler(unsigned long scause, unsigned long sepc);

void interrupt_handler(unsigned long scause, unsigned long sepc);

void trap_handler(unsigned long scause, unsigned long sepc);
 
#endif
