#ifndef __TASK1_H__
#define __TASK1_H__
//FIXME :: change naming
#include <sys/types.h>
#include <sys/process.h>

typedef struct
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3;
    // What about r12, r13, r14, r15??
} Registers;

typedef struct fd
{
    int alloted;
    int file_sz;
    int num_bytes_read;
    void* file_ptr;
    void* last_matched_header;
    int is_dir;
    int ref_count;
}fd;

typedef struct Task
{
    Registers regs; // how to assign kstack?
    struct Task *next;
    struct Task *prev;
    fd fd_array[100];
    uint64_t *pml4;
    uint64_t *cr3;
    struct vma *vma_pntr;
    int pid;
    int ppid;
    int is_foregrnd;
    struct Task * parent_task;
} Task;

void init_tasks();
void init_tasks_0_3();
void createTask(Task *, void(*)(), uint64_t, uint64_t *);
void yield(); // Switch kernel tasks
void yield_0_3(); // Switch from kernel to user space
void switchTask(Registers *old, Registers *new_task); // The function which actually switches
void createTask1(Task *task, uint64_t virtual_address, uint64_t flags);
void init_tasks1();


#endif /* __TASK_H__ */
