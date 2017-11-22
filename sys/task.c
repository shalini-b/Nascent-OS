//FIX ME :: change this code
//reference os dev

#include <sys/task.h>
#include <sys/page.h>
#include <sys/kprintf.h>
#include <sys/types.h>
static Task *runningTask;
static Task mainTask;
static Task otherTask1,otherTask2;
void
contextswitch(Registers *, Registers *);
void
ring_0_3_switch(Registers *, Registers *);

static void
otherMain1()
{
    kprintf("Hello multitasking world11111!\n");
    //runningTask = &otherTask1;
    while(1)
    {
        yield();
        kprintf("Hello multitasking world11111!\n");
    }

}


static void
otherMain2()
{
    kprintf("Hello multitasking world22222!\n");
    yield();
    kprintf("Hello multitasking world22222!\n");
    while(1)
    {
        yield();
        kprintf("Hello multitasking world222222!\n");
    }
}

static void
otherMain3()
{
    kprintf("Hello multitasking world22222!\n");
    yield_0_3();

}

void
init_tasks()
{
    // Get flags and CR3
    __asm__ __volatile__ ("movq %%cr3, %%rax; movq %%rax, %0;":"=m"(mainTask.regs.cr3)::"%rax");
    __asm__ __volatile__ ("pushfq; movq (%%rsp), %%rax; movq %%rax, %0; popfq;":"=m"(mainTask.regs.flags)::"%rax");

    createTask(&otherTask1, otherMain1, mainTask.regs.flags, (uint64_t *) mainTask.regs.cr3);
    createTask(&otherTask2, otherMain2, mainTask.regs.flags, (uint64_t *) mainTask.regs.cr3);
    otherTask1.next = &otherTask2;
    otherTask2.next = &otherTask1;
    mainTask.next = &otherTask1;
    runningTask = &mainTask;
}

void user_mode_test()
{
    kprintf("inside user mode\n");
    while(1)
    {

    }
}

void
init_tasks_0_3()
{
    // Get flags and CR3
    __asm__ __volatile__ ("movq %%cr3, %%rax; movq %%rax, %0;":"=m"(mainTask.regs.cr3)::"%rax");
    __asm__ __volatile__ ("pushfq; movq (%%rsp), %%rax; movq %%rax, %0; popfq;":"=m"(mainTask.regs.flags)::"%rax");

    createTask(&otherTask1, otherMain3, mainTask.regs.flags, (uint64_t *) mainTask.regs.cr3);
    createTask(&otherTask2, user_mode_test, mainTask.regs.flags, (uint64_t *) mainTask.regs.cr3);
    otherTask1.next = &otherTask2;
    otherTask2.next = &otherTask1;
    mainTask.next = &otherTask1;
    runningTask = &mainTask;
}

void
createTask(Task *task, void (*main)(), uint64_t flags, uint64_t *pagedir)
{
    //rax,rbx,rcx,rdx,rsi,rdi,rsp,rbp,rip,flags,cr3
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.rip = (uint64_t) main;;
    task->regs.flags = flags;
    task->regs.cr3 = (uint64_t) pagedir;
    task->regs.rsp = (uint64_t) page_alloc() + (0x1000);
    task->next = 0;
}

void
yield()
{
    Task *last = runningTask;
    runningTask = runningTask->next;
    contextswitch(&last->regs, &runningTask->regs);
}


void
yield_0_3()
{
    Task *last = runningTask;
    runningTask = runningTask->next;
    ring_0_3_switch(&last->regs, &runningTask->regs);
}