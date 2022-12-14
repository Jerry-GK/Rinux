.equ reg_size, 0x8    
    
    .section .text.entry
    .align 2
    .globl _traps 
_traps:
    # YOUR CODE HERE
    # 1. save 32 registers and sepc to stack
    csrw sscratch, sp 
    addi sp, sp, -32*reg_size

    sd x1, 0*reg_size(sp)
    sd x3, 2*reg_size(sp)
    sd x4, 3*reg_size(sp)
    sd x5, 4*reg_size(sp)
    sd x6, 5*reg_size(sp)
    sd x7, 6*reg_size(sp)
    sd x8, 7*reg_size(sp)
    sd x9, 8*reg_size(sp)
    sd x10, 9*reg_size(sp)
    sd x11, 10*reg_size(sp)
    sd x12, 11*reg_size(sp)
    sd x13, 12*reg_size(sp)
    sd x14, 13*reg_size(sp)
    sd x15, 14*reg_size(sp)
    sd x16, 15*reg_size(sp)
    sd x17, 16*reg_size(sp)
    sd x18, 17*reg_size(sp)
    sd x19, 18*reg_size(sp)
    sd x20, 19*reg_size(sp)
    sd x21, 20*reg_size(sp)
    sd x22, 21*reg_size(sp)
    sd x23, 22*reg_size(sp)
    sd x24, 23*reg_size(sp)
    sd x25, 24*reg_size(sp)
    sd x26, 25*reg_size(sp)
    sd x27, 26*reg_size(sp)
    sd x28, 27*reg_size(sp)
    sd x29, 28*reg_size(sp)
    sd x30, 29*reg_size(sp)
    sd x31, 30*reg_size(sp)

    csrr t0, sepc
    sd t0, 31*reg_size(sp)

    # 2. call trap_handler (sscratch ??) (pass a0, a1 as parameters)
    csrrw t0, sscratch, x0
    sd t0, 1*reg_size(sp)

    csrr a0, scause
    csrr a1, sepc
    mv a2, sp

    call trap_handler

    #3. restore sepc and 32 registers(x2(sp) should be restore last)from stack
    ld t0, 31*reg_size(sp)
    csrw sepc, t0

    ld x1, 0*reg_size(sp)
    ld x3, 2*reg_size(sp)
    ld x4, 3*reg_size(sp)
    ld x5, 4*reg_size(sp)
    ld x6, 5*reg_size(sp)
    ld x7, 6*reg_size(sp)
    ld x8, 7*reg_size(sp)
    ld x9, 8*reg_size(sp)
    ld x10, 9*reg_size(sp)
    ld x11, 10*reg_size(sp)
    ld x12, 11*reg_size(sp)
    ld x13, 12*reg_size(sp)
    ld x14, 13*reg_size(sp)
    ld x15, 14*reg_size(sp)
    ld x16, 15*reg_size(sp)
    ld x17, 16*reg_size(sp)
    ld x18, 17*reg_size(sp)
    ld x19, 18*reg_size(sp)
    ld x20, 19*reg_size(sp)
    ld x21, 20*reg_size(sp)
    ld x22, 21*reg_size(sp)
    ld x23, 22*reg_size(sp)
    ld x24, 23*reg_size(sp)
    ld x25, 24*reg_size(sp)
    ld x26, 25*reg_size(sp)
    ld x27, 26*reg_size(sp)
    ld x28, 27*reg_size(sp)
    ld x29, 28*reg_size(sp)
    ld x30, 29*reg_size(sp)
    ld x31, 30*reg_size(sp)
    
    ld x2, 1*reg_size(sp)

    # 4. return from trap
    sret


    .extern dummy
    .global __dummy
__dummy:
    # YOUR CODE HERE
    la t0, dummy
    csrw sepc, t0
    sret


    .globl __switch_to
__switch_to:
    # save state to prev process
    # YOUR CODE HERE
    # a0: prev
    # a1: next
    # bias is based on the structure of task_struct
    sd ra, 5*reg_size(a0)
    sd sp, 6*reg_size(a0)
    sd s0, 7*reg_size(a0)
    sd s1, 8*reg_size(a0)
    sd s2, 9*reg_size(a0)
    sd s3, 10*reg_size(a0)
    sd s4, 11*reg_size(a0)
    sd s5, 12*reg_size(a0)
    sd s6, 13*reg_size(a0)
    sd s7, 14*reg_size(a0)
    sd s8, 15*reg_size(a0)
    sd s9, 16*reg_size(a0)
    sd s10, 17*reg_size(a0)
    sd s11, 18*reg_size(a0)

    # restore state from next process
    # YOUR CODE HERE
    ld ra, 5*reg_size(a1)
    ld sp, 6*reg_size(a1)
    ld s0, 7*reg_size(a1)
    ld s1, 8*reg_size(a1)
    ld s2, 9*reg_size(a1)
    ld s3, 10*reg_size(a1)
    ld s4, 11*reg_size(a1)
    ld s5, 12*reg_size(a1)
    ld s6, 13*reg_size(a1)
    ld s7, 14*reg_size(a1)
    ld s8, 15*reg_size(a1)
    ld s9, 16*reg_size(a1)
    ld s10, 17*reg_size(a1)
    ld s11, 18*reg_size(a1)
    ret
