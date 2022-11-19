#include "defs.h"
#include "string.h"
#include "mm.h"

#include "printk.h"

extern char _ekernel[];

struct {
    struct run *freelist;
} kmem;

uint64 kalloc() {
    struct run *r;

    r = kmem.freelist;
    kmem.freelist = r->next;
    
    memset((void *)r, 0x0, PGSIZE);
    return (uint64) r;
}

void kfree(uint64 addr) {
    struct run *r;

    // PGSIZE align 
    addr = addr & ~(PGSIZE - 1);

    memset((void *)addr, 0x0, (uint64)PGSIZE);

    r = (struct run *)addr;
    r->next = kmem.freelist;
    kmem.freelist = r;

    return ;
}

void kfreerange(char *start, char *end) {
    char *addr = (char *)PGROUNDUP((uint64)start);
    // printk("here\n");
    // printk("addr = %x\n", (uint64)addr);
    // printk("pe = %x\n", PHY_END);
    // printk("end = %x\n", (uint64)end);
    // printk("rest = %x\n", ((uint64)end - (uint64)addr));
    int pnum = 0;
    for (; (uint64)(addr) + PGSIZE <= (uint64)end; addr += PGSIZE) {
        pnum++;
        //printk("Allocated pages: %d. Rest memory to allocate : %ld KB\n", pnum, ((uint64)end - (uint64)addr) / 1024);
        kfree((uint64)addr);
    }
}

void mm_init(void) {
    kmem.freelist = NULL;
    uint64 vm_bound = VM_START + PHY_SIZE;
    kfreerange(_ekernel, (char *)(vm_bound));
    printk("[Initialize] Memory initialization done!\n");
}
