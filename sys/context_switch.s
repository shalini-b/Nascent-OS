
.section .text
.global contextswitch
contextswitch:
    #rax,rbx,rcx,rdx,rsi,rdi,rsp,rbp,rip,flags,cr3,r12, r13, r14, r15, krsp;
    #0    8   16  24  32  40  48  56  64  72    80  88   96  104  112  120
    #old function
    #backup rbx
    movq %rbx, 8(%rdi)
    #get rip
    popq %rbx
    movq %rbx, 64(%rdi)
    #backup other registers
    movq %rax,  (%rdi)
    movq %rcx, 16(%rdi)
    movq %rdx, 24(%rdi)
    movq %rsi, 32(%rdi)
    movq %rdi, 40(%rdi)
    movq %rsp, 120(%rdi)
    movq %rbp, 56(%rdi)
    #flags
    pushfq
    popq %rbx
    movq %rbx, 72(%rdi)
    #cr3
    movq %cr3,%rbx
    movq %rbx,80(%rdi)


     #rax,rbx,rcx,rdx,rsi,rdi,rsp,rbp,rip,flags,cr3;
     #0    8   16  24  32  40  48  56  64  72    80
    #new function
    #load registers
    movq    (%rsi),%rax
    movq  8(%rsi),%rbx
    movq  16(%rsi),%rcx
    movq  24(%rsi),%rdx
    #fixme :: can we load these registers rsi & rdi

    movq  120(%rsi),%rsp
    movq  56(%rsi),%rbp
    #for rip
    movq  64(%rsi),%rbx
    pushq  %rbx
    #flags
    movq  72(%rsi),%rbx
    pushq %rbx
    popfq
    movq 80(%rsi), %rbx
    movq %rbx, %cr3
    #restore rbx
    movq   8(%rsi),%rbx
    ret

