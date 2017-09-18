#include <sys/kprintf.h>
#include <sys/idt.h>

void timer_int()
{
    static int timer_count = 0;
    static int time_from_reboot = 0;
    if (timer_count == 18){
        timer_count = -1;
        time_from_reboot++;
        print_time(time_from_reboot);
    }
    timer_count++;
    outb(0x20, 0x20);
}
