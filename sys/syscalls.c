#include <sys/syscall.h>
#include <sys/kprintf.h>
#include <sys/process.h>

extern Task *RunningTask;

uint64_t
syscall_handler(Registers1 *regs)
{
    int syscall_num = regs->rax;

    switch (syscall_num)
    {

        case SYS_write:
        {
            regs->rax = write_to_console((uint64_t)regs->rdi, (char*)regs->rsi, (int)regs->rdx);
            break;
        }
        case SYS_fork:
        {
            int child_pid = fork_process(RunningTask);
            if (RunningTask->kstack[511] == 123456) {
                RunningTask->kstack[511] = 0;
                regs->rax = 0;
            }
            else {
                regs->rax = child_pid;
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