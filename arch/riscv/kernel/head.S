#include "defs.h"
.extern setup_vm_prepare
.extern pgtbl_prepare
.extern setup_vm_map
.extern mm_init
.extern task_init

.extern start_kernel
    .section .text.init
    .globl _start

_start:
    la sp, boot_stack_top

    call setup_vm_prepare
    call relocate
    call mm_init # initialize physical memory
    call setup_vm_map
    //sfence.vma zero, zero

    # set stvec = _traps (Direct，直接就是异常处理函数)
    la t0, _traps
    csrw stvec, t0

    # set sie[STIE] = 1 (开启S模式下时钟中断的响应)
    csrr t0, sie
    ori t0, t0, 0x20
    csrw sie, t0

    # set first time interrupt
    rdtime t0
    li t1, 10000000
    add a0, t0, t1
    add a6, zero, zero
    add a7, zero, zero
    ecall

    # set sstatus[SIE] = 1 (开启S模式下中断响应)
    csrr t0, sstatus
    ori t0, t0, 0x2
    csrw sstatus, t0

    call task_init # initialize task threads
    # goto start_kernel
    call start_kernel

relocate:
    # set ra = ra + VA_PA_OFFSET
    # set sp = sp + VA_PA_OFFSET (If you have set the sp before)
    li t0, VA_PA_OFFSET
    add ra, ra, t0
    add sp, sp, t0

    # set satp with pgtbl_prepare
    la t0, pgtbl_prepare
    srli t0, t0, 12
    li t1, 0x8000000000000000
    or t0, t0, t1
    csrw satp, t0

    # flush tlb
    sfence.vma zero, zero
    ret

# - your lab1 code -
    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096# <-- change to your stack size
    .globl boot_stack_top
boot_stack_top:
