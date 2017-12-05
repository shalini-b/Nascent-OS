#include <sys/syscall.h>
#include <sys/kprintf.h>
#include<sys/task.h>


//void syscall_handler() {
//   kprintf("In write syscall handler");
//}


void syscall_handler(Registers1 * regs) {
    int syscall_num = regs->rax;

    switch(syscall_num) {

        case SYS_write: {
            kprintf("\n In write syscall handle\n");
            int res = 1;
            regs->rax = res;
            break;
        }
    }
}
