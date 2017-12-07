#include <sys/syscall.h>
#include <sys/kprintf.h>
#include<sys/process.h>


//void syscall_handler() {
//   kprintf("In write syscall handler");
//}

//
//void syscall_handler(Registers1 * regs) {
//    int syscall_num = regs->rax;
//
//    switch(syscall_num) {
//
//        case SYS_write: {
//            kprintf("\n In write syscall handle\n");
//            int res = 1;
//            regs->rax = res;
//            break;
//        }
//    }
//}



uint64_t
syscall_handler(Registers1 *regs)
{
    int syscall_num = regs->rax;

    switch (syscall_num)
    {

        case SYS_write:
        {
//            kprintf("hello");
            regs->rax = write_to_console((uint64_t)regs->rdi,(char*)regs->rsi,(int)regs->rdx);
        }

    }
    return 0;
}

uint64_t
write_to_console(uint64_t fd, char *buffer, uint64_t count)
{
    for (int i = 0; i < count; i++)
    {
        kprintf("%c", buffer[i]);
    }
    return 0;
}