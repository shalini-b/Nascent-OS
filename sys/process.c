#include <sys/defs.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/page.h>
#include <sys/virmem.h>
#include <sys/memset.h>
#include <sys/tarfs.h>
// FIXME: get corresponding sys file
#include <strings.h>

struct vma * free_vma_head = NULL;
struct vma vma_arr[NUM_VMA];

extern uint64_t *kpml_addr;

Task pcb_arr[NUM_PCB];
Task * free_pcb_head = NULL;

static char args[10][100];

// FIXME: call this somewhere in paging
void initialise_vma() {
    for (int i = NUM_VMA-1; i >= 0; i--) {
        if (free_vma_head==NULL) {
            free_vma_head = &vma_arr[i];
            free_vma_head->next = NULL;
            free_vma_head->prev = NULL;
        }
        else {
            free_vma_head->prev = &vma_arr[i];
            vma_arr[i].next = free_vma_head;
            free_vma_head = &vma_arr[i];
        }
    }
}

struct vma * get_free_vma() {
    if (free_vma_head == NULL) {
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma * free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;
    return free_vma;
}

struct vma * fetch_free_vma(uint64_t start_addr, uint64_t end_addr, uint64_t vm_flags, uint64_t vmtype) {
    if (free_vma_head==NULL) {
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma * free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;
    // initialise
    free_vma->next = NULL;
    free_vma->prev = NULL;
    free_vma->start_addr = start_addr;
    free_vma->end_addr = end_addr;
    free_vma->vm_flags = vm_flags;
    free_vma->vmtype = vmtype;

    return free_vma;
}

// Creating a linked list on top of the list of
// PCB's in kernel space for free PCBs
void create_pcb_list() {
    for (int i = NUM_PCB-1; i >= 0; i--) {
        if (free_pcb_head==NULL) {
            free_pcb_head = &pcb_arr[i];
            free_pcb_head->next = NULL;
            free_pcb_head->prev = NULL;
        }
        else {
            free_pcb_head->prev = &pcb_arr[i];
            pcb_arr[i].next = free_pcb_head;
            free_pcb_head = &pcb_arr[i];
        }
    }
}

uint64_t * create_new_pml_table() {
    // FIXME: is it enough??
    uint64_t * pml_addr = (uint64_t *) page_alloc();
    kprintf("In create proc %p", pml_addr);
    pml_addr[511] = kpml_addr[511];
    return pml_addr;
}

struct Task * fetch_free_pcb() {
    if (free_pcb_head==NULL) {
        kprintf("Uh Oh! No PCB's left!!");
        return NULL;
    }
    struct Task * free_pcb = free_pcb_head;
    free_pcb_head = free_pcb_head->next;
    // Initiating new MM struct
    struct mm_struct * proc_mm = (struct mm_struct *) page_alloc();
    // proc_mm->pml4 = create_new_pml_table();
    proc_mm->vma_head = NULL;
    proc_mm->count = 0;

    // Initiating process vars
    free_pcb->task_mm = proc_mm;
    // CAUTION: check if it works
    free_pcb->regs.cr3 = ((uint64_t) create_new_pml_table()) - KERNBASE;
    free_pcb->next = NULL;
    free_pcb->prev = NULL;
    free_pcb->parent_task = NULL;
    //free_pcb->childnode = NULL;
    //free_pcb->sib = NULL;
    //free_pcb->num_child = 0;

    // FIXME: check this value for errors
    free_pcb->pid = (free_pcb - pcb_arr)/ sizeof(Task);
    free_pcb->ppid = 0;
    free_pcb->task_state = READY;

    // FIXME: do we need to memset before reuse of PCB?
    // FIXME: memset regs values?
    // memset((void*)free_pcb->kstack, 0, KSTACK_SIZE);
    // memset((void*)free_pcb->fd_array, 0, MAX_FDS * sizeof(fd));

    // initiate 0, 1, 2 fd for each pcb
    for (int i = 0; i < 3; i++) {
        free_pcb->fd_array->fdtype = STD_FD;
        free_pcb->fd_array->alloted= 1;
        free_pcb->fd_array->file_ptr = NULL;
        free_pcb->fd_array->file_sz = 0;
        free_pcb->fd_array->num_bytes_read = 0;
        free_pcb->fd_array->is_dir = 0;
        free_pcb->fd_array->last_matched_header = NULL;
    }

    return free_pcb;
}

int fork_process(Task * parent_pcb) {
    Task * child_pcb = fetch_free_pcb();
    child_pcb->parent_task = parent_pcb;
    child_pcb->ppid = parent_pcb->pid;
    // FIXME: check if copy works
    str_copy(child_pcb->filename, parent_pcb->filename);

    // copying file desc
    for (int i = 0; i < MAX_FDS; i++) {
        // FIXME: check if copy works properly
        child_pcb->fd_array[i] = parent_pcb->fd_array[i];
        // FIXME: increment fd for parent if child is pointing to it??
        // parent_pcb->fd_array[i].alloted++;
    }

    // Backup CR3 values
    uint64_t ppml4 = parent_pcb->regs.cr3;
    uint64_t cpml4 = child_pcb->regs.cr3;

    // Copy mm struct except for vma_list
    memcopy((void*)parent_pcb->task_mm, (void*)child_pcb->task_mm, sizeof(struct mm_struct));
    child_pcb->task_mm->vma_head = NULL;

    /*if (parent_pcb->childnode) {
        child_pcb->sib = parent_pcb->childnode;
    }
    parent_pcb->childnode = child_pcb;
    parent_pcb->num_child++;
     */

    // Copy VMA structs
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

        // copy page table entries
        // Caution - these may not be required
        uint64_t end = (uint64_t) ScaleUp((uint64_t *) parent_vma->end_addr);
        uint64_t start = (uint64_t) ScaleDown((uint64_t *) parent_vma->start_addr);
        int cnt = (end - start) / PAGE_SIZE;

        for (int i = 0; i < cnt; i++) {
            uint64_t *page_phyaddr = NULL;
            // check if parent page tables have a mapping for given viraddr
            // CAUTION: check if we have to align or not
            int present = get_pte_entry(ppml4, start + i * PAGE_SIZE, page_phyaddr);
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
                set_mapping(cpml4, start + i * PAGE_SIZE, *page_phyaddr, 0);
                page_addr->ref_count++;
            }
        }

        parent_vma = parent_vma->next;
    }
    invalidate_tlb((uint64_t) ppml4);

    // TODO: update kernel stack values

    // parent returns this
    return child_pcb->pid;
}

void copy_arg_to_stack(uint64_t *user_stack, int argc)
{
    // order is 0, envp, 0, argv, argv pointers, argc
    uint64_t *argv[10];
    // Mark beginning of stack, leave first entry or mark it as 0
    user_stack = user_stack - 0x8;
    // Store the argument values
    for (int i = argc-1; i >= 0; i--) {
        int arg_len = len(args[i]) + 1;
        user_stack = (uint64_t *) ((void *) user_stack - arg_len);
        memcopy(args[i], (char*)user_stack, arg_len);
        argv[i] = user_stack;
    }

    // Store the argument pointers
    for (int i = argc-1; i >= 0; i--) {
        user_stack--;
        *user_stack = (uint64_t) argv[i];
    }

    // Store the arg count
    user_stack--;
    *user_stack = (uint64_t) argc;
}

void sys_execvpe(char *filename, char *argv[], char *envp[])
{
    // FIXME: use kmalloc or page_alloc
    uint64_t * stack_start = (uint64_t *)page_alloc() + PAGE_SIZE;
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
    copy_arg_to_stack(stack_start, argc);
    // Anything else to be retained? fd_array??
    // Exit from the current process
    clean_task_for_exec(RunningTask);

    // FIXME: handle envp
    uint64_t bin_viradd = load_elf(RunningTask, filename, argv);

    // copy new stack to RunningTask's stack vma
    start_viraddr = (uint64_t) USTACK;
    end_address = (uint64_t) (USTACK - USTACK_SIZE);
    struct vma* tmp = RunningTask->task_mm->vma_head;
    while(tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = fetch_free_vma(start_viraddr, end_address, RW, STACK);
    RunningTask->task_mm->count++;

    // FIXME: Map all addresses from USTACK till USTACK_SIZE
    // set mapping for stack to USTACK in pcb's PML
    uint64_t proc_pml4 = (uint64_t) (RunningTask->regs.cr3 - KERNBASE);
    uint64_t stack_phyaddr = (uint64_t) ((stack_start - PAGE_SIZE) - KERNBASE);
    set_mapping(proc_pml4, USTACK, stack_phyaddr, 7);


    // Enable interrupt for scheduling next process
    // FIXME: set rip, rsp of pcb for new process and then iretq

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
    memset((void*)cur_task->fd_array, 0, sizeof(fd)*MAX_FDS);
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

