#ifndef __TASK1_H__
#define __TASK1_H__
//FIXME :: change naming
#include <sys/types.h>

void
init_tasks();
void
init_tasks_0_3();
typedef struct
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3;
//uint64_t rbx, rbp, rdi, rsi, rsp, r12, r13, r14, r15, cr3,flags,rip;
} Registers;

typedef struct fd
{
    int alloted;
    int file_sz;
    int num_bytes_read;
    void* file_ptr;
}fd;

typedef struct Task
{
    Registers regs;
    struct Task *next;
    fd fd_array[100];
} Task;

void
createTask(Task *, void(*)(), uint64_t, uint64_t *);

void
yield(); // Switch task frontend
void
yield_0_3();
void
switchTask(Registers *old, Registers *new_task); // The function which actually switches
void
createTask1(Task *task, uint64_t virtual_address, uint64_t flags);
void
init_tasks1();


#endif /* __TASK_H__ */