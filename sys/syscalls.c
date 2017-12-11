#include <sys/syscall.h>
#include <sys/kprintf.h>
#include<sys/task.h>
#include<sys/tarfs.h>
#include<sys/types.h>


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
            regs->rax = (uint64_t)write_to_console((uint64_t)regs->rdi,(char*)regs->rsi,(int)regs->rdx);
            break;
        }
        case SYS_open_dir:
        {
            regs->rax = open_dir((char *)regs->rdi);
            break;
        }
        case SYS_read_dir:
        {
            regs->rax = read_dir((int)regs->rdi,(char*)regs->rsi);
            break;
        }
        case SYS_close_dir:
        {
            regs->rax = close_dir((int)regs->rdi);
            break;
        }
        case SYS_open:
        {
            regs->rax = open_s((char *)regs->rdi,(int)regs->rsi);
            break;
        }
        case SYS_read:
        {
            regs->rax = read_s((int)regs->rdi,(char*)regs->rsi,(int)regs->rdx);
            break;
        }
        case SYS_close:
        {
            regs->rax = close_s((int)regs->rdi);
            break;
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

