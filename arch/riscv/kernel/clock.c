// clock.c
#include "clock.h"
#include "sbi.h"

// QEMU��ʱ�ӵ�Ƶ����10MHz, Ҳ����1�����൱��10000000��ʱ�����ڡ�
unsigned long TIMECLOCK = 10000000;

unsigned long get_cycles() {
    // ʹ�� rdtime ��д������࣬��ȡ time �Ĵ����� (Ҳ����mtime �Ĵ��� )��ֵ������
    unsigned long num;
    asm volatile("rdtime %0" : "=r"(num));
    return num; 
}

void clock_set_next_event() {
    // ��һ�� ʱ���ж� ��ʱ���
    unsigned long next = get_cycles() + TIMECLOCK;
    printk("Next interruption cycle: %d\n", next);

    // ʹ�� sbi_ecall ����ɶ���һ��ʱ���жϵ�����
    sbi_ecall(SBI_SETTIMER, 0, next, 0, 0, 0, 0, 0);
} 
