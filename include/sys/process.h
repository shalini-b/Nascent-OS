#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <sys/types.h>

//FIXME :: change naming

// FIXME: vma sufficient??
#define NUM_VMA 5000
#define NUM_PCB 1000
#define MAX_FDS 10
#define KSTACK_SIZE 512

// FIXME: set these properly
enum vmatype {
    TEXT,
    DATA,
    HEAP,
    STACK,
    ANON
};

enum vmaflag {
    NONE,  //no permission
    X,
    W,
    WX,
    R,
    RX,
    RW,
    RWX
};

struct vma {
    uint64_t start_addr;
    uint64_t end_addr;
    struct vma * next;
    // used to build global array
    struct vma * prev;
    uint64_t vm_flags;
    uint64_t vmtype;
    uint64_t p_filesz;
    uint64_t tarfs_base;
};

struct mm_struct {
    struct vma *vma_head;
    int count;
    // FIXME: keep the brk fields here to support mmap?
    uint64_t begin_stack;
    uint64_t argv_start, argv_end;
};

typedef struct {
    // NOTE: rsp is user rsp, krsp is for kernel stack
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3, r12, r13, r14, r15, krsp;
    // What about r12, r13, r14, r15??
} Registers;

typedef struct {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, r12, r13, r14, r15;
}Registers1;


typedef enum {
    STD_FD = 1,
    OTHER = 2
} FD_TYPE;


typedef enum {
    // process running
    RUNNING,
    // process ready for execution
    READY,
    // process in sleep
    SLEEP,
    // process waiting for input
    WAIT,
    // not used anywhere till now
    IDLE,
    // process is exiting
    EXIT,
    // child running, parent exited
    ZOMBIE,
    // process is waiting for child
    SUSPENDED
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
    Registers regs;
    char filename[75];
    struct mm_struct * task_mm;
    uint64_t kstack[KSTACK_SIZE];
    struct Task *next;
    struct Task *prev;
    fd fd_array[MAX_FDS];
    int pid;
    int ppid;
    struct Task * parent_task;
    int sleep_sec;
    TASK_STATES task_state;
    char cwd[75];
} Task;

Task* RunningTask;

// FIXME: refactor these calls
void init_tasks();
void init_tasks_0_3();
void createTask(Task *, void(*)(), uint64_t, uint64_t *);
void yield(); // Switch kernel tasks
void yield_0_3(); // Switch from kernel to user space
void switchTask(Registers *old, Registers *new_task); // The function which actually switches
void createTask1(Task *task, uint64_t virtual_address, uint64_t flags);
void init_tasks1();


int fork_process();
uint64_t *copy_arg_to_stack(uint64_t *user_stack, int argc, char *envp[]);
void report_error(char* msg);
void clean_task_for_exec(Task *cur_task);
void sys_execvpe(char *file, char *argv[], char *envp[]);
void schedule();
void return_to_user();
void sys_exit();


#endif
