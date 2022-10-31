.extern start_kernel
    .section .text.init
    .globl _start
_start:
    # YOUR CODE HERE
    la sp, boot_stack_top

    call mm_init # initialize physical memory

    call task_init # initialize task threads

    # set stvec = _traps (Direct，直接就是异常处理函数)
    la t0, _traps
    csrw stvec, t0

    # set sie[STIE] = 1 (开启S模式下时钟中断的响应)
    csrr t0, sie
    ori t0, t0, 0x20
    csrw sie, t0

    # set first time interrupt  ???
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

    # goto start_kernel
    jal start_kernel

    # - your lab1 code -
    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096# <-- change to your stack size
    .globl boot_stack_top
boot_stack_top:
