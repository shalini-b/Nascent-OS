#include <sys/defs.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/page.h>
#include <sys/virmem.h>
#include <sys/memset.h>
#include <sys/tarfs.h>
#include <sys/gdt.h>
#include <sys/strings.h>
#include <sys/proc_mngr.h>

void contextswitch(Registers *, Registers *);
static char args[10][100];
extern int is_first_proc;


int fork_process() {
    Task * parent_pcb = RunningTask;
    Task * child_pcb = fetch_free_pcb();
    str_copy(parent_pcb->filename, child_pcb->filename);
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
            int present = get_pte_entry(ppml4, start + i * PAGE_SIZE, &page_phyaddr);
            // if any of the intermediary table values are not present, then just skip the viraddr
            if (present == 1) {
                // Used to increment page ref count
                struct page * page_addr = get_page_from_PA(page_phyaddr);
                // if physical page present, update the W & COW for physical page pointer
                // FIXME: only set COW for writeable pages?
                UNSET_WRITE(page_phyaddr);
                SET_COW(page_phyaddr);
                set_mapping(ppml4, start + i * PAGE_SIZE, page_phyaddr, 0);
                // the flags are already set, so just pass the value to be set in child page table
                set_mapping(cpml4, start + (i * PAGE_SIZE), page_phyaddr, 0);
                page_addr->ref_count++;
            }
        }

        parent_vma = parent_vma->next;
    }
    invalidate_tlb((uint64_t) ppml4);

    // copy all contents of kstack to child
    memcpy_uint(&(parent_pcb->kstack[0]), &(child_pcb->kstack[0]), KSTACK_SIZE);
    // Keep a magic number at the 511th entry of child kstack to recognise child in syscall handler
    child_pcb->kstack[511] = 10101;
    // Add child to task list
    add_to_task_list(child_pcb);

    uint64_t cur_rsp;
    __asm__ __volatile__ (
        "movq %%rsp, %0 \n\t"
        :"=r" (cur_rsp)
        ::);
    // update child kernel stack for RIP & RSP registers
    child_pcb->regs.rip = *((uint64_t *)cur_rsp + 17);
    child_pcb->regs.krsp = (uint64_t) (((uint64_t)(&(child_pcb->kstack[509]))) - (uint64_t)(((uint64_t)&(RunningTask->kstack[509])) - cur_rsp) );

    // parent returns this
    return child_pcb->pid;
}

uint64_t *copy_arg_to_stack(uint64_t *user_stack, int argc, char *envp[])
{
    // order is envp, argv, 0, envp pointers, 0, argv pointers, argc
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
            user_stack = user_stack - arg_len;
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
    return user_stack;
}

void sys_execvpe(char *filename, char *argv[], char *envp[])
{
    memset(&args[0], 0, 1000);
    uint64_t stack_base = (uint64_t)page_alloc();
    uint64_t stack_start = (stack_base + PAGE_SIZE);

    // reorganise arguments
    int argc = 0;
    if (argv) {
        while (argv[argc] != NULL) {
            str_copy(argv[argc], args[argc]);
            argc++;
        }
    }

    // copy args to stack
    stack_start = (uint64_t) copy_arg_to_stack((uint64_t *)stack_start, argc, envp);

    if (is_first_proc != 1) {
        // Exit from the current process
        clean_task_for_exec(RunningTask);
    }
    else if (is_first_proc == 1) {
        is_first_proc = 0;
    }

    uint64_t bin_viradd = load_elf(RunningTask, filename, argv);

    // Set RIP & RSP for new process
    RunningTask->regs.rip = bin_viradd;

    // copy new stack to RunningTask's stack vma
    uint64_t start_viraddr = (uint64_t) (USTACK - USTACK_SIZE);
    uint64_t end_address = (uint64_t) USTACK;
    struct vma* tmp = RunningTask->task_mm->vma_head;
    while(tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = fetch_free_vma(start_viraddr, end_address, RW, STACK,0,0);
    RunningTask->task_mm->count++;

    // set mapping for stack to USTACK in pcb's PML
    // All other pages will be given to USTACK on the fly
    uint64_t proc_pml4 = (uint64_t) (RunningTask->regs.cr3 + KERNBASE);
    uint64_t stack_phyaddr = stack_base - KERNBASE;
    set_mapping(proc_pml4, USTACK-PAGE_SIZE, stack_phyaddr, 7);

    // assign stack address which points to argc
    RunningTask->regs.rsp = (uint64_t) USTACK -  (((uint64_t)PAGE_SIZE) - (((uint64_t)stack_start) - stack_base));
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
    // FIXME: clean regs??
    // reset task vars
    cur_task->next        = NULL;
    cur_task->prev        = NULL;
    cur_task->sleep_sec = 0;

    // memset filename
    memset((void*)cur_task->filename, 0, 75);

    // clean mm struct
    cur_task->task_mm->vma_head = NULL;
    cur_task->task_mm->count = 0;
    cur_task->task_mm->begin_stack = 0;

    // re-initiate all fds for each pcb
    for (int i = 0; i < MAX_FDS; i++)
    {
        if (i<3) {
            cur_task->fd_array[i].fdtype = STD_FD;
            cur_task->fd_array[i].alloted = 1;
        }
        else {
            cur_task->fd_array[i].fdtype = OTHER;
            cur_task->fd_array[i].alloted = 0;
        }
        cur_task->fd_array[i].file_ptr = NULL;
        cur_task->fd_array[i].file_sz = 0;
        cur_task->fd_array[i].num_bytes_read = 0;
        cur_task->fd_array[i].is_dir = 0;
        cur_task->fd_array[i].last_matched_header = NULL;
    }
    // clean kstack
    memset((void*)cur_task->kstack, 0, KSTACK_SIZE);

    // FIXME: deep clean the page tables & vma & mm structs
    // For now, just clear out the PML table
    uint64_t * proc_pml = (uint64_t *) (cur_task->regs.cr3 + KERNBASE);
    memset((void*)proc_pml, 0, 511*8);
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
    add_to_task_list(last);
    set_tss_rsp((void*)&RunningTask->kstack[509]);
    contextswitch(&last->regs, &RunningTask->regs);
}

void sys_exit() {

    if (str_compare(RunningTask->filename, "bin/sbush") == 0) {
        kprintf("Sbush exiting..");
    }
    RunningTask->task_state = ZOMBIE;

    // Wake up the parent of this task if it was waiting
    Task* parent = RunningTask->parent_task;
    if (parent != NULL) {
        if (parent->task_state == SUSPENDED) {
            parent->task_state = READY;
        }
    }

    // Mark init as parent for all children of this Running Task
    // FIXME: Irrespective of their task state?
    for (int i = 0; i < NUM_PCB; i++) {
        Task tmp = pcb_arr[i];
        if(tmp.ppid == RunningTask->pid)
        {
            tmp.ppid = INIT_TASK->pid;
        }
    }
}
