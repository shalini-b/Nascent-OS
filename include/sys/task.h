#ifndef __TASK_H__
#define __TASK_H__
//FIXME :: change naming
#include <sys/types.h>

 void initTasking();

typedef struct {
    uint64_t rbx, rbp, rdi, rsi, rsp, r12, r13, r14, r15, cr3,flags,rip;
} Registers;

typedef struct Task {
    Registers regs;
    struct Task *next;
} Task;

 void initTasking();
 void createTask(Task*, void(*)(), uint64_t, uint64_t*);

 void yield(); // Switch task frontend
 void switchTask(Registers *old, Registers *new_task); // The function which actually switches

#endif /* __TASK_H__ */