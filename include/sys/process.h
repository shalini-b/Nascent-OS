#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <sys/types.h>
//FIXME :: change naming

#define NUM_VMA 100
#define NUM_PCB 1000
#define MAX_FDS 10
#define KSTACK_SIZE 512

extern Task* RunningTask;

// FIXME: remove what is not needed
typedef enum vmatype {
    TEXT,
    DATA,
    HEAP,
    STACK,
    ANON,
    FILETYPE,
    NONE
} VMA_TYPES;

typedef enum vmaflag {
    NONE,  //no permission
    X,     //execute only
    W,     //write only
    WX,    //write execute
    R,     //read only
    RX,    //read execute
    RW,    //read write
    RWX
} VM_FLAGS;

struct vma {
    uint64_t start_addr;
    uint64_t end_addr;
    struct vma * next;
    struct mm_struct *proc_mm;
    VM_FLAGS vm_flags;
    VMA_TYPES vmtype;
};

struct mm_struct {
    struct vma *vma_head;
    int count;
    uint64_t *pml4;
    uint64_t begin_stack;
    uint64_t argv_start, argv_end;
};

typedef struct rgs
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3;
    // What about r12, r13, r14, r15??
} Registers;

typedef enum {
    STD_FD = 1
} FD_TYPE;

typedef enum {
    READY = 0
}TASK_STATES;

typedef struct fd
{
    int alloted;  // FIXME: Can it take a value greater than 1?
    int file_sz;
    int num_bytes_read;
    void* file_ptr;
    void* last_matched_header;
    int is_dir;
    FD_TYPE fdtype;
}fd;

typedef struct Task
{
    Registers regs; // how to assign kstack?
    char filename[75];
    struct mm_struct * task_mm;
    uint64_t rip;
    uint64_t rsp;
    uint64_t kstack[KSTACK_SIZE];
    struct Task *next;
    struct Task *prev;
    fd fd_array[MAX_FDS];
    uint64_t *cr3;
    int pid;
    int ppid;
    struct Task * parent_task;
    struct Task * childnode;
    struct Task * sib;
    int num_child;
    TASK_STATES task_state;
} Task;

void init_tasks();
void init_tasks_0_3();
void createTask(Task *, void(*)(), uint64_t, uint64_t *);
void yield(); // Switch kernel tasks
void yield_0_3(); // Switch from kernel to user space
void switchTask(Registers *old, Registers *new_task); // The function which actually switches
void createTask1(Task *task, uint64_t virtual_address, uint64_t flags);
void init_tasks1();

void initialise_vma();
struct vma * fetch_free_vma();
void create_pcb_list();
struct Task * fetch_free_pcb();
int fork_process();
int create_pid();
//void create_process();

#endif
