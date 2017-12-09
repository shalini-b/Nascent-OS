#include <sys/syscall.h>
#include <sys/kprintf.h>
#include <sys/process.h>


uint64_t
syscall_handler(Registers1 *regs)
{
    // CAUTION - Do not use any variables here!!
    // Only use regs
    // FIXME: Can i mess with r15 here? if not, create a temp syscall_num in registers
/*    __asm__ __volatile__(
    "movq %%rax, %0 \n\t"
    :"=r" (regs->r15)::);
*/
    switch (regs->rax)
    {
        case SYS_write:
        {
            regs->rax = write_to_console((uint64_t)regs->rdi, (char*)regs->rsi, (int)regs->rdx);
            break;
        }
        case SYS_fork:
        {
            // CAUTION - Schedule is not called for fork.
            // It will return back to the same process
            int child_pid = (uint64_t) fork_process();

            if (RunningTask->kstack[511] == 10101) {
                RunningTask->kstack[511] = 0;
                 return 0;
            }
            else {
                return child_pid;
            }
            break;
        }
        case SYS_getpid:
        {
            regs->rax = RunningTask->pid;
            break;
        }
        case SYS_getppid:
        {
            regs->rax = RunningTask->ppid;
            break;
        }
        default:{
            // FIXME: check this
            regs->rax = 0;
        }

    }
    schedule();
    return regs->rax;
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

