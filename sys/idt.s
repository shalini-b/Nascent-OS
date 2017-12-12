.extern gen_int_handler
.global isr_handler

isr_handler:
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
    pushq %rsp
    movq %rsp,%rdi
    call gen_int_handler
    popq %rsp
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
    // sti
    iretq

.extern page_fault_handler
.global pgfault

pgfault:
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
    pushq %rsp
    movq 0x80(%rsp), %rdi
    call page_fault_handler
    popq %rsp
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
    add $8, %rsp
    //sti
    iretq

.extern int_handler13
.global int13

int13:
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
    pushq %rsp
    movq %rsp,%rdi
    call int_handler13
    popq %rsp
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
    //sti
    iretq

