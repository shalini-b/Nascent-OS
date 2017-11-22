#include "sys/task.h"
#include "sys/kprintf.h"

void doIt() {
    while(1)
    {
        kprintf("Switching to task2... \n");
        yield();
        kprintf("switching to task 1!\n");
    }

}

