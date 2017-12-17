
.extern syscall_handler
.global sys_int
#TODO :: all registers backup

sys_int:
    cli
    pushq $128
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %rbp
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax
    movq %rsp, %rdi
    callq syscall_handler
    popq %rbx
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rbp
    popq %rbp
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    add $0x08, %rsp
    // sti
    iretq

