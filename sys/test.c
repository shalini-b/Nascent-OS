#include "sys/task.h"
#include "sys/kprintf.h"

void doIt() {
    kprintf("Switching to otherTask... \n");
    yield();
    kprintf("Returned to mainTask!\n");
}