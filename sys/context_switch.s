
.section .text
.global contextswitch
contextswitch:
    #old function
    movq %rbx, (%rdi)
    popq %rbx
    movq %rbx, 88(%rdi)
    movq %rbp, 8(%rdi)
    movq %rdi, 16(%rdi)
    movq %rsi, 24(%rdi)
    movq %rsp, 32(%rdi)
    movq %r12, 40(%rdi)
    movq %r13, 48(%rdi)
    movq %r14, 56(%rdi)
    movq %r15, 64(%rdi)
    movq %cr3,%rbx
    movq %rbx,72(%rdi)
    pushfq
    popq %rbx
    movq %rbx, 80(%rdi)


    #new function
    movq    (%rsi),%rbx
    movq  8(%rsi),%rbp
    movq  16(%rsi),%rdi
    movq  24(%rsi),%rsi
    movq  32(%rsi),%rsp
    movq  40(%rsi),%r12
    movq  48(%rsi),%r13
    movq  56(%rsi),%r14
    movq  64(%rsi),%r15
    movq  72(%rsi),%rbx
    movq  %rbx,%cr3
    movq  80(%rsi),%rbx
    pushq %rbx
    popfq
    movq  88(%rsi),%rbx
    #load new function
    movq  %rbx,(%rsp)
    #restore rbx
        movq    (%rsi),%rbx
    ret

