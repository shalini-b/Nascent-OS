#include <sys/syscall.h>
#include <sys/kprintf.h>
#include <sys/process.h>
#include <sys/tarfs.h>
#include <sys/types.h>
#include <sys/process.h>
#include <sys/proc_mngr.h>


uint64_t
syscall_handler(Registers1 *regs)
{
    // CAUTION - use of variables here might alter the functionality of child!

    switch (regs->rax)
    {
        case SYS_write:
        {
            regs->rax = (uint64_t)write_to_console((uint64_t)regs->rdi,(char*)regs->rsi,(int)regs->rdx);
            break;
        }
        case SYS_fork:
        {
            // It will return back to the same process
            int child_pid = (uint64_t) fork_process();

            if (RunningTask->kstack[511] == 10101) {
                RunningTask->kstack[511] = 0;
                for (int i=0; i<18; i++) {
                    __asm__ volatile("popq %%rax":::"%rax");
                }
                schedule();
                return 0;
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
            RunningTask->task_state = SUSPENDED;
            break;
        }
        case SYS_exit:
        {
            sys_exit();
            break;
        }
        case SYS_ps:
        {
            sys_ps();
            break;
        }
        case SYS_sleep_s:
        {
            RunningTask->sleep_sec = (int)regs->rdi ;
            RunningTask->task_state = SLEEP;
            break;
        }
        case SYS_kill_s :
        {
            // FIXME: implement this
            break;
        }
        case SYS_chdir_s:
        {
            set_cwd((char *)regs->rdi);
            break;
        }
        case  SYS_getcwd_s:
        {
            fetch_cwd((char *)regs->rdi);
            break;
        }
        case SYS_execve: {
            sys_execvpe((char *)regs->rdi,(char **)regs->rsi,(char **)regs->rdx);
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

void sys_ps() {
    Task* p = overall_task_list;
    // FIXME: show only selected process states
    char *a[8] = {"RUNNING", "READY", "SLEEP", "WAIT FOR INPUT", "IDLE", "EXIT", "ZOMBIE", "WAITING"};
    kprintf("PID   PROCESS      MODE \n");
    while(p!=NULL)
    {
        kprintf("%d     %s           %s\n",p->pid,p->filename,(char*) a[p->task_state]);
        p=p->next;
    }
    kprintf("%d     %s         %s\n",RunningTask->pid,RunningTask->filename, (char*) a[RunningTask->task_state]);
}
