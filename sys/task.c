#include <sys/task.h>
#include <sys/page.h>
#include <sys/kprintf.h>
#include <sys/types.h>

static Task *runningTask;
static Task mainTask;
static Task otherTask;
void contextswitch(Registers *,Registers *);
 
static void otherMain() {
    kprintf("Hello multitasking world!");
    yield();
}
 
void init_tasks() {
    // Get flags and CR3
    __asm__ __volatile__ ("movq %%cr3, %%rax; movq %%rax, %0;":"=m"(mainTask.regs.cr3)::"%rax");
    __asm__ __volatile__ ("pushfq; movq (%%rsp), %%rax; movq %%rax, %0; popfq;":"=m"(mainTask.regs.flags)::"%rax");
 
    createTask(&otherTask, otherMain, mainTask.regs.flags, (uint64_t *)mainTask.regs.cr3);
    mainTask.next = &otherTask;
    otherTask.next = &mainTask;
 
    runningTask = &mainTask;
}
 
void createTask(Task *task, void (*main)(), uint64_t flags, uint64_t *pagedir) {
    task->regs.rbx = 0;
    task->regs.rbp = 0;
    task->regs.rdi= 0;
    task->regs.rsi = 0;
    task->regs.rsp = 0;
    task->regs.r12 = 0;
    task->regs.r13 = 0;
    task->regs.r14 = 0;
    task->regs.r15 = 0;
    task->regs.flags = flags;
    task->regs.rip = (uint64_t) main;
    task->regs.cr3 = (uint64_t) pagedir;
    task->regs.rsp = (uint64_t) page_alloc() + 0x1000;
    task->next = 0;
}
 
void yield() {
    Task *last = runningTask;
    runningTask = runningTask->next;
    contextswitch(&last->regs, &runningTask->regs);
}
