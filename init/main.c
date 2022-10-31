#include "printk.h"
#include "sbi.h"
#include "defs.h"
#include "mm.h"
#include "proc.h"

extern void test();

int start_kernel() {
    
    printk("\n[BOOT] Hello Rinux!\n");

    test(); // DO NOT DELETE !!!

	return 0;
}
