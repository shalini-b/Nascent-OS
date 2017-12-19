#include <sys/kprintf.h>
#include <sys/idt.h>
#include <sys/idt.h>
#include <sys/process.h>
#include <sys/proc_mngr.h>

void timer_int()
{
    static int timer_count = 0;
    static int time_from_reboot = 0;
    if (timer_count == 18){
        timer_count = -1;
        time_from_reboot++;
        print_time(time_from_reboot);

        // Iterate over the complete running task list
        // and decrement the sleep secs for waiting tasks
        Task* p = overall_task_list;
        while(p!=NULL)
        {
            if(p->task_state == SLEEP)
            {
                p->sleep_sec--;
                if (p->sleep_sec == 0)
                {
                   p->task_state = READY;
                }
            }
            p = p->next;
        }
    }
    timer_count++;

    outb(0x20, 0x20);
    // FIXME: call sys_yield
}
