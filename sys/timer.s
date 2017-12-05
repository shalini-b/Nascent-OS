.extern timer_int
.global timer
/*
timer:
    cli
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    call timer_int
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rbp
    sti
    iretq


 timer:
     cli
     pushq %rax
     pushq %rcx
     pushq %rdx
     pushq %r8
     pushq %r9
     pushq %r10
     pushq %r11
     call timer_int
     popq %r11
     popq %r10
     popq %r9
     popq %r8
     popq %rdx
     popq %rcx
     popq %rax
     sti
     iretq*/


timer:
    cli
    pushq %rdi
    pushq %rsi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    call timer_int
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rsi
    popq %rdi
    #sti
    iretq

