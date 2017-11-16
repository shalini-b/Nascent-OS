#include <sys/task.h>
#include <sys/virmem.h>
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
    //rax,rbx,rcx,rdx,rsi,rdi,rsp,rbp,rip,flags,cr3
    task->regs.rax =0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx= 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.rip = (uint64_t) main;;
    task->regs.flags = flags;
    task->regs.cr3 = (uint64_t) pagedir;
    task->regs.rsp = (uint64_t) fetch_free_page_cr3() + (0x1000);
    task->next = 0;
}

void yield() {
    Task *last = runningTask;
    runningTask = runningTask->next;
    contextswitch(&last->regs, &runningTask->regs);
}
