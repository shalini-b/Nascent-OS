//FIX ME :: change this code
//reference os dev
#include <sys/types.h>
#include <sys/gdt.h>
#include <sys/page.h>
#include <sys/kprintf.h>
#include <sys/types.h>
#include <sys/tarfs.h>
#include <sys/page.h>
#include <sys/memset.h>
#include <sys/proc_mngr.h>
#include <sys/process.h>

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
    kprintf("reached kernal task 1!\n");
//    print_elf_file("bin/sbush");
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
    RunningTask = &mainTask;
}

void user_mode_test()
{
    kprintf("inside user mode\n");
    while(1);
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
    RunningTask = &mainTask;
}

void
init_tasks1()
{
    // Get flags and CR3
    __asm__ __volatile__ ("movq %%cr3, %%rax; movq %%rax, %0;":"=m"(mainTask.regs.cr3)::"%rax");
    __asm__ __volatile__ ("pushfq; movq (%%rsp), %%rax; movq %%rax, %0; popfq;":"=m"(mainTask.regs.flags)::"%rax");
    Task * cur_pcb = fetch_free_pcb();
    char *tmp[] = {"bin/sbush", "3"};
    uint64_t virtual_address = load_elf(cur_pcb, "bin/sbush", tmp);
    set_tss_rsp((void*)&cur_pcb->kstack[509]);
    cur_pcb->regs.krsp = (uint64_t)&cur_pcb->kstack[509];
    cur_pcb->regs.rip = virtual_address;
    cur_pcb->regs.rsp = ((uint64_t) page_alloc()) + (0x1000);
    RunningTask = cur_pcb;
    long output; \
    __asm__ __volatile__(
                            "pushq $35 \n\t" \
                            "pushq %2 \n\t" \
                            "pushfq \n\t"\
                             "pushq $43\n\t"\
                            "pushq %1 \n\t" \
                            "iretq\n\t"
    :"=r" (output)  :"r"((uint64_t) (cur_pcb->regs.rip )) ,"r"( (uint64_t) (cur_pcb->regs.rsp)));\
}

void
createTask1(Task *task, uint64_t virtual_address, uint64_t flags)
{
    //rax,rbx,rcx,rdx,rsi,rdi,rsp,rbp,rip,flags,cr3
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.rip = virtual_address;
    task->regs.flags = flags;
    uint64_t rsp = ((uint64_t) page_alloc()) + (0x1000)-16;
    kprintf("rsp value %p",rsp);
    set_tss_rsp((void*)rsp);
//    task->regs.cr3 = (uint64_t) pagedir;
    task->regs.rsp = (uint64_t) page_alloc() + (0x1000);
    task->next = 0;
//    __asm__ __volatile__("sti");
    long output; \
    __asm__ __volatile__(
                            "pushq $35 \n\t" \
                            "pushq %2 \n\t" \
                            "pushfq \n\t"\
                             "pushq $43\n\t"\
                            "pushq %1 \n\t" \
                            "iretq\n\t"
                             :"=r" (output)  :"r"((uint64_t) (task->regs.rip )) ,"r"( (uint64_t)(task->regs.rsp)));\
/*//    \
//    long output; \
//    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
//                            "movq %2, %%rdi \n\t" \
//                            "movq %3, %%rsi \n\t" \
//                            "movq %4, %%rdx \n\t" \
//                            "movq %5, %%r10 \n\t" \
//                            "movq %6, %%r8 \n\t" \
//                            "movq %7, %%r9 \n\t" \
//                            "syscall\n\t" \
//                            "movq %%rax, %0 \n\t" \
//                             :"=r" (output) \
//                             :"r" ((long)(__NR_##name)), \
//                             "r" ((uint64_t)(arg1)), \
//                             "r" ((uint64_t)(arg2)), \
//                             "r" ((uint64_t)(arg3)), \
//                             "r" ((uint64_t)(arg4)), \
//                             "r" ((uint64_t)(arg5)), \
//                             "r" ((uint64_t)(arg6)) \
//                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9"); \
//    return (type) output; \*/
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
    task->regs.rsp = ((uint64_t) page_alloc()) + (0x1000);
    task->next = 0;
}

void
yield()
{
    Task *last = RunningTask;
    RunningTask = RunningTask->next;
    contextswitch(&last->regs, &RunningTask->regs);
}


void
yield_0_3()
{
    Task *last = RunningTask;
    RunningTask = RunningTask->next;
    // FIXME: Set TSS properly
    uint64_t rsp = ((uint64_t) page_alloc()) + (0x1000) - 8;
    set_tss_rsp((void*)rsp);
    ring_0_3_switch(&last->regs, &RunningTask->regs);
}

