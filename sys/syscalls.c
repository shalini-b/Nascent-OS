#include <sys/syscall.h>
#include <sys/kprintf.h>
#include <sys/process.h>
#include<sys/tarfs.h>
#include<sys/types.h>
#include <sys/process.h>
#include <sys/proc_mngr.h>
#include <sys/pit.h>


uint64_t
syscall_handler(Registers1 *regs)
{
    // CAUTION - use of variables here might alter the functionality of child!

/*    __asm__ __volatile__(
    "movq %%rax, %0 \n\t"
    :"=r" (regs->rax)::);
*/
    switch (regs->rax)
    {
        case SYS_write:
        {
            regs->rax = (uint64_t)write_to_console((uint64_t)regs->rdi,(char*)regs->rsi,(int)regs->rdx);
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
            // break;
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
        case SYS_wait_s:
        {
            RunningTask->task_state = WAIT;
            schedule();
            break;
        }
        case SYS_exit:
        {
            RunningTask->task_state = ZOMBIE;
            Task* p_t = RunningTask->parent_task;
            if(p_t->task_state == WAIT)
            {
                p_t->task_state=READY;
                schedule();
            }
            break;
        }
        case SYS_ps:
        {
            Task* p = overall_task_list;
            while(p!=NULL)
            {
                kprintf("%d %s %d",p->pid,p->filename,p->task_state);
                p=p->next;
            }
            break;
        }
        case SYS_sleep_s:
        {
            RunningTask->sleep_sec = (int)regs->rdi ;
            Task* p = RunningTask;
            p->task_state = SLEEP;
            schedule();
            break;
        }
        default: {
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

