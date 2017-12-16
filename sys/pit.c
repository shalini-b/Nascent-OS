#include <sys/kprintf.h>
#include <sys/idt.h>
#include <sys/idt.h>
#include <sys/pit.h>
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

        if(RunningTask->sleep_sec !=0)
        {
            RunningTask->sleep_sec--;
        }
        else
        {
            Task* p = overall_task_list;
            while(p!=NULL)
            {
                if(p->task_state==SLEEP)
                {
                    p->task_state = READY;
                }
                p=p->next;
            }
            RunningTask->sleep_sec = 0;

        }
    }
    timer_count++;

    outb(0x20, 0x20);
}
