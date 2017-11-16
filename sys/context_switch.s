
.section .text
.global contextswitch
contextswitch:
    #rax,rbx,rcx,rdx,rsi,rdi,rsp,rbp,rip,flags,cr3;
    #0    8   16  24  32  40  48  56  64  72    80
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
    movq %rsp, 48(%rdi)
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
    movq  32(%rsi),%rsi#fixme :: can we load these registers rsi & rdi
    movq  40(%rsi),%rdi
    movq  48(%rsi),%rsp
    movq  56(%rsi),%rbp
    #for rip
    movq  64(%rsi),%rbx
    movq  %rbx,(%rsp)
    #flags
    movq  72(%rsi),%rbx
    pushq %rbx
    popfq
    #restore rbx
    movq   8(%rsi),%rbx
    ret

