#include <sys/defs.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/page.h>
#include <sys/virmem.h>
#include <sys/memset.h>
#include <sys/tarfs.h>
#include <sys/gdt.h>
// FIXME: get corresponding sys file
#include <strings.h>
#include <sys/proc_mngr.h>

void contextswitch(Registers *, Registers *);
static char args[10][100];
extern int is_first_proc;


int fork_process() {
    Task * parent_pcb = RunningTask;
    Task * child_pcb = fetch_free_pcb();
    child_pcb->parent_task = parent_pcb;
    child_pcb->ppid = parent_pcb->pid;
    // FIXME: filename followed by zeros, guess it is not a problem
    str_copy(child_pcb->filename, parent_pcb->filename);

    // copying file desc
    for (int i = 0; i < MAX_FDS; i++) {
        // FIXME: check if copy works properly
        child_pcb->fd_array[i] = parent_pcb->fd_array[i];
        // FIXME: increment fd for parent if child is pointing to it??
        // parent_pcb->fd_array[i].alloted++;
    }

    // Backup CR3 values
    uint64_t ppml4 = parent_pcb->regs.cr3 + KERNBASE;
    uint64_t cpml4 = child_pcb->regs.cr3 + KERNBASE;

    // Copy mm struct except for vma_list
    memcopy((void*)parent_pcb->task_mm, (void*)child_pcb->task_mm, sizeof(struct mm_struct));
    child_pcb->task_mm->vma_head = NULL;

    /*if (parent_pcb->childnode) {
        child_pcb->sib = parent_pcb->childnode;
    }
    parent_pcb->childnode = child_pcb;
    parent_pcb->num_child++;
     */

    // Deep Copy VMA structs
    struct vma * child_vmas = NULL;
    struct vma * parent_vma = parent_pcb->task_mm->vma_head;
    while (parent_vma) {

        if (child_pcb->task_mm->vma_head == NULL) {
            child_pcb->task_mm->vma_head = get_free_vma();
            *(child_pcb->task_mm->vma_head) = *parent_vma;
            child_vmas = child_pcb->task_mm->vma_head;
        } else {
            child_vmas->next = get_free_vma();
            *(child_vmas->next) = *parent_vma;
            child_vmas = child_vmas->next;
        }

        // deep copy page table entries
        uint64_t end = (uint64_t) ScaleUp((uint64_t *) parent_vma->end_addr);
        uint64_t start = (uint64_t) ScaleDown((uint64_t *) parent_vma->start_addr);
        int cnt = (end - start) / PAGE_SIZE;

        for (int i = 0; i < cnt; i++) {
            uint64_t page_phyaddr = 0;
            // check if parent page tables have a mapping for given viraddr
            // CAUTION: check if we have to align viraddr or not
            int present = get_pte_entry(ppml4, start + i * PAGE_SIZE, &page_phyaddr);
            // if any of the intermediary table values are not present, then just skip the viraddr
            if (present == 1) {
                // Used to increment page ref count
                struct page * page_addr = get_page_from_PA(page_phyaddr);
                // if physical page present, update the W & COW for physical page pointer
                // FIXME: only set COW for writeable pages?
                UNSET_WRITE(page_phyaddr);
                SET_COW(page_phyaddr);
                // CAUTION: check flags again & check if we have to align or not
                // the flags are already set, so just pass the value to be set in child page table
                set_mapping(cpml4, start + i * PAGE_SIZE, page_phyaddr, 0);
                page_addr->ref_count++;
            }
        }

        parent_vma = parent_vma->next;
    }
    invalidate_tlb((uint64_t) ppml4);
    
    // copy all contents of kstack to child
    memcpy_uint(&parent_pcb->kstack[0], &child_pcb->kstack[0], KSTACK_SIZE);
    // Keep a magic number at the 511th entry of child kstack to recognise child in syscall handler
    child_pcb->kstack[511] = 10101;
    uint64_t cur_rsp;
    __asm__ __volatile__ (
        "movq %%rsp, %0 \n\t"
        :"=r" (cur_rsp)
        ::);
    // TODO: update child kernel stack for RIP & RSP registers
    child_pcb->regs.rip = *((uint64_t *)cur_rsp + 15);
    child_pcb->regs.krsp = (uint64_t) ((&child_pcb->kstack[509]) - 20);

    // Add child to task list
    add_to_task_list(child_pcb);

    // parent returns this
    return child_pcb->pid;
}

void copy_arg_to_stack(uint64_t *user_stack, int argc, char *envp[])
{
    // FIXME: order is envp, argv, 0, envp pointers, 0, argv pointers, argc
    uint64_t *argv[10], *argv1[10];
    // Mark beginning of stack, leave first entry or mark it as 0
    // memset this stack - done in page_alloc
    user_stack = user_stack - 0x8;

    // Count the envp variables
    int num_envp = 0;
    if (envp) {
        while (envp[num_envp]) {
            num_envp++;
        }
    }

    // Store the envp values
    if (envp) {
        for (int i = num_envp - 1; i >= 0; i--) {
            int arg_len = len(envp[i]) + 1;
            user_stack = (uint64_t * )((void *) user_stack - arg_len);
            memcopy((void *)envp[i], (void *) user_stack, arg_len);
            argv1[i] = user_stack;
        }
    }

    // Store the argument values
    for (int i = argc - 1; i >= 0; i--) {
        int arg_len = len(args[i]) + 1;
        user_stack = (uint64_t * )((void *) user_stack - arg_len);
        memcopy((void *)args[i], (void *) user_stack, arg_len);
        argv[i] = user_stack;
    }

    // place a 0
    user_stack = user_stack - 0x8;

    // Store envp pointers
    if (envp) {
        for (int i = num_envp - 1; i >= 0; i--) {
            user_stack--;
            *user_stack = (uint64_t) argv1[i];
        }
    }

    // place a 0
    user_stack = user_stack - 0x8;

    // Store the argument pointers
    for (int i = argc - 1; i >= 0; i--) {
        user_stack--;
        *user_stack = (uint64_t) argv[i];
    }

    // Store the arg count
    user_stack--;
    *user_stack = (uint64_t) argc;
}

void sys_execvpe(char *filename, char *argv[], char *envp[])
{
    // FIXME: use kmalloc or page_alloc??
    uint64_t * stack_start = (uint64_t *)page_alloc() + PAGE_SIZE;
    // NOTE - memset pages given to stack & heap
    // reorganise arguments
    int argc = 1;
    str_copy(filename, args[0]);
    if (argv) {
        while (argv[argc]) {
            str_copy(argv[argc], args[argc+1]);
            argc++;
        }
    }
    // copy args to stack
    copy_arg_to_stack(stack_start, argc, envp);

    if (is_first_proc != 1) {
        // Anything else to be retained? fd_array??
        // Exit from the current process
        clean_task_for_exec(RunningTask);
    }
    else if (is_first_proc == 1) {
        is_first_proc = 0;
    }

    // FIXME: handle envp
    uint64_t bin_viradd = load_elf(RunningTask, filename, argv);

    // Set RIP & RSP for new process
    RunningTask->regs.rip = bin_viradd;
    // CAUTION - assign stack address which points to argc
    RunningTask->regs.rsp = (uint64_t) stack_start;

    // copy new stack to RunningTask's stack vma
    uint64_t start_viraddr = (uint64_t) USTACK;
    uint64_t end_address = (uint64_t) (USTACK - USTACK_SIZE);
    struct vma* tmp = RunningTask->task_mm->vma_head;
    while(tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = fetch_free_vma(start_viraddr, end_address, RW, STACK);
    RunningTask->task_mm->count++;

    // set mapping for stack to USTACK in pcb's PML
    // All other pages will be given to USTACK on the fly
    uint64_t proc_pml4 = (uint64_t) (RunningTask->regs.cr3 + KERNBASE);
    uint64_t stack_phyaddr = (uint64_t) ((stack_start - PAGE_SIZE) - KERNBASE);
    set_mapping(proc_pml4, USTACK, stack_phyaddr, 7);

    // iretq to user mode
    return_to_user();
}

void return_to_user() {
    // set rip, rsp of pcb for new process and then iretq
    set_tss_rsp((void*)&RunningTask->kstack[509]);
    RunningTask->regs.krsp = (uint64_t) &RunningTask->kstack[509];
    __asm__ __volatile__(
        "pushq $35 \n\t" \
        "pushq %1 \n\t" \
        "pushfq \n\t"\
        "pushq $43 \n\t"\
        "pushq %0 \n\t" \
        "iretq \n\t"
    ::"r"((uint64_t) (RunningTask->regs.rip )),
    "r" ((uint64_t) (RunningTask->regs.rsp)));\
}

void clean_task_for_exec(Task *cur_task) {
    // reset task vars
    cur_task->next        = NULL;
    cur_task->prev        = NULL;
    cur_task->parent_task      = NULL;
    cur_task->task_state = READY;
    // FIXME: memset filename??

    // cur_task->child_nodehead   = NULL;
    // cur_task->sib    = NULL;
    // cur_task->num_child = 0;

    // FIXME: clean regs??

    // clean mm struct
    cur_task->task_mm->vma_head = NULL;
    cur_task->task_mm->count = 0;
    cur_task->task_mm->begin_stack = 0;
    // CAUTION - initiate others

    // clean * initiate the std fds
    memset((void*)cur_task->fd_array, 0, sizeof(fd) * MAX_FDS);
    // initiate 0, 1, 2 fd for each pcb
    for (int i = 0; i < 3; i++) {
        cur_task->fd_array->fdtype = STD_FD;
        cur_task->fd_array->alloted= 1;
        cur_task->fd_array->file_ptr = NULL;
        cur_task->fd_array->file_sz = 0;
        cur_task->fd_array->num_bytes_read = 0;
        cur_task->fd_array->is_dir = 0;
        cur_task->fd_array->last_matched_header = NULL;
    }
    // clean kstack
    memset((void*)cur_task->kstack, 0, KSTACK_SIZE);
}

void report_error(char* msg)
{
    kprintf("\n[FAULT]: %s!", msg);
    while(1);
}

void schedule() {
    // take backup
    Task *last = RunningTask;
    // get the next ready task
    // incoming task state marked as running
    // Existing task state marked to ready
    RunningTask = fetch_ready_task();
    // FIXME: uncomment this later
    // add_to_task_list(last);
    contextswitch(&last->regs, &RunningTask->regs);
}

