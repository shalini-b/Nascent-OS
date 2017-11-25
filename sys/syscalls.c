#include <sys/syscall.h>
#include <sys/kprintf.h>

void syscall_handler(Registers * regs) {
    int syscall_num = regs->rax;

    switch(syscall_num) {

        case SYS_write: {
            kprintf("In write syscall handler");
            int res = 1;
            regs->rax = res;
            break;
        }
    }
}