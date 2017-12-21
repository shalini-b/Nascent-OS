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

struct vma *free_vma_head = NULL;
Task *free_pcb_head = NULL;

void
initialise_vma()
{
    for (int i = NUM_VMA - 1; i >= 0; i--)
    {
        if (free_vma_head == NULL)
        {
            free_vma_head = &vma_arr[i];
            free_vma_head->next = NULL;
            free_vma_head->prev = NULL;
        }
        else
        {
            free_vma_head->prev = &vma_arr[i];
            vma_arr[i].next = free_vma_head;
            free_vma_head = &vma_arr[i];
        }
    }
}

struct vma *
get_free_vma()
{
    if (free_vma_head == NULL)
    {
        // FIXME: how to handle in a better way?
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma *free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;
    free_vma->next = NULL;
    free_vma->prev = NULL;
    return free_vma;
}

struct vma *
fetch_free_vma(uint64_t start_addr, uint64_t end_addr, uint64_t vm_flags, uint64_t vmtype, uint64_t fsz, uint64_t base)
{
    if (free_vma_head == NULL)
    {
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma *free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;
    // initialise
    free_vma->next = NULL;
    free_vma->prev = NULL;
    free_vma->start_addr = start_addr;
    free_vma->end_addr = end_addr;
    free_vma->vm_flags = vm_flags;
    free_vma->vmtype = vmtype;
    free_vma->p_filesz = fsz;
    free_vma->tarfs_base = base;

    return free_vma;
}

// Creating a linked list on top of the list of
// PCB's in kernel space for free PCBs
void
create_pcb_list()
{
    for (int i = NUM_PCB - 1; i >= 0; i--)
    {
        if (free_pcb_head == NULL)
        {
            free_pcb_head = &pcb_arr[i];
            free_pcb_head->next = NULL;
            free_pcb_head->prev = NULL;
        }
        else
        {
            free_pcb_head->prev = &pcb_arr[i];
            pcb_arr[i].next = free_pcb_head;
            free_pcb_head = &pcb_arr[i];
        }
    }
}

uint64_t *
create_new_pml_table()
{
    uint64_t *pml_addr = (uint64_t *) page_alloc();
    pml_addr[511] = kpml_addr[511];
    return pml_addr;
}

struct Task *
fetch_free_pcb()
{
    if (free_pcb_head == NULL)
    {
        kprintf("Uh Oh! No PCB's left!!");
        return NULL;
    }
    struct Task *free_pcb = free_pcb_head;
    free_pcb_head = free_pcb_head->next;
    // Initiating new MM struct
    struct mm_struct *proc_mm = (struct mm_struct *) page_alloc();
    proc_mm->vma_head = NULL;
    proc_mm->count = 0;

    // Initiating process vars
    free_pcb->task_mm = proc_mm;
    free_pcb->regs.cr3 = ((uint64_t) create_new_pml_table()) - KERNBASE;
    free_pcb->regs.krsp = (uint64_t)(&free_pcb->kstack[509]);
    free_pcb->next = NULL;
    free_pcb->prev = NULL;
    free_pcb->parent_task = NULL;

    free_pcb->pid = ((uint64_t) free_pcb - (uint64_t) & pcb_arr[0]) / sizeof(Task);
    free_pcb->ppid = -1;
    free_pcb->task_state = READY;

    // FIXME: Set Cur work dir for new process
    str_copy(" ", free_pcb->cwd);

    // FIXME: do we need to memset before reuse of PCB?
    // FIXME: memset regs values?
    // memset((void*)free_pcb->kstack, 0, KSTACK_SIZE);

    // initiate all fds for each pcb
    for (int i = 0; i < MAX_FDS; i++)
    {
        if (i<3) {
            free_pcb->fd_array[i].fdtype = STD_FD;
            free_pcb->fd_array[i].alloted = 1;
        }
        else {
            free_pcb->fd_array[i].fdtype = OTHER;
            free_pcb->fd_array[i].alloted = 0;
        }
        free_pcb->fd_array[i].file_ptr = NULL;
        free_pcb->fd_array[i].file_sz = 0;
        free_pcb->fd_array[i].num_bytes_read = 0;
        free_pcb->fd_array[i].is_dir = 0;
        free_pcb->fd_array[i].last_matched_header = NULL;
    }

    return free_pcb;
}

void clear_pcb(Task *cur_task) {
    // reset task vars
    // CAUTION - pid not reset
    cur_task->next        = NULL;
    cur_task->prev        = NULL;
    cur_task->parent_task = NULL;
    cur_task->sleep_sec = 0;
    cur_task->task_state = READY;
    cur_task->ppid = -1;

    // memset filename
    memset((void*)cur_task->filename, 0, 75);

    // FIXME: free memory given to vma_head & its members
    // check if vma_head present

    // clean mm struct
    cur_task->task_mm->vma_head = NULL;
    cur_task->task_mm->count = 0;
    cur_task->task_mm->begin_stack = 0;
    cur_task->task_mm->argv_start = 0;
    cur_task->task_mm->argv_end = 0;

    // FIXME: free memory given to task_mm

    // FIXME: free memory given to fd_array & its members

    // FIXME: clear out all fds - check this
    memset((void*)cur_task->fd_array, 0, MAX_FDS * sizeof(fd));

    // clean kstack
    memset((void*)cur_task->kstack, 0, KSTACK_SIZE);

    // CAUTION - check this
    // deep clean the page tables
    uint64_t proc_pml = (uint64_t) (cur_task->regs.cr3 + KERNBASE);
    clear_mapping(proc_pml);
    
    // FIXME: clean regs??
}

// ********** Process scheduling helper methods ********

void
idle_process()
{
    while (1)
    {
        __asm__ __volatile__("sti;");
        __asm__ __volatile__("hlt;");
        __asm__ __volatile__("cli;");
        schedule();
    }
}

uint64_t wait_for_child()
{
    for (int i = 0; i< NUM_PCB; i++)
    {
        Task* child = &pcb_arr[i];
        if(child->task_state == ZOMBIE &&
           child->ppid == RunningTask->pid)
        {
            // CAUTION - check this
            clear_pcb(child);
            append_in_free_list(child);
            return i;
        }
    }

    RunningTask->task_state = SUSPENDED;
    schedule();
    for (int i = 0; i< NUM_PCB; i++)
    {
        Task* child = &pcb_arr[i];
        if(child->task_state == ZOMBIE &&
           child->ppid == RunningTask->pid)
        {
            // CAUTION - check this
            clear_pcb(child);
            append_in_free_list(child);
            return i;
        }
    }

    return 0;
}

void init_process()
{
    while (1)
    {
        wait_for_child();
        clean_child_pcb();
    }
}

void clean_child_pcb()
{
    for (int i = 0; i< NUM_PCB; i++)
    {
        Task* child = &pcb_arr[i];
        if(child->task_state == ZOMBIE &&
           child->ppid == RunningTask->pid)
        {
            // CAUTION - check this
            clear_pcb(child);
            append_in_free_list(child);
        }
    }
}

void
create_idle_process( )
{
    // CAUTION - This is a kernel process
    IDLE_TASK = fetch_free_pcb();
    IDLE_TASK->task_state = READY;
    IDLE_TASK->regs.rip =(uint64_t)&idle_process;
    str_copy("Idle", IDLE_TASK->filename);
    add_to_task_list(IDLE_TASK);
}

void
create_init_process( )
{
    // CAUTION - This is a kernel process
    INIT_TASK = fetch_free_pcb();
    INIT_TASK->task_state = READY;
    INIT_TASK->regs.rip =(uint64_t)&init_process;
    str_copy("Init", INIT_TASK->filename);
    add_to_task_list(INIT_TASK);
}

void
append_in_free_list(Task *task)
{
    // FIXME - REMOVE THIS
    //task->task_state = UNAVAIL;
    //return;

    // Append the given PCB at the end of free list
    // maintaining the pcb position still because of array
    // if free list is empty
    if (free_pcb_head == NULL)
    {
        free_pcb_head = task;
        task->next = NULL;
        return;
    }

    // Iterate to the last and append
    Task *temp = free_pcb_head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    // Maintain the next and prev pointers
    temp->next = task;
    task->next = NULL;
    task->prev = temp;

    return;

}

void
add_to_task_list(Task *task)
{
    // Mark a running task as ready if it was running
    if (task->task_state == RUNNING)
    {
        task->task_state = READY;
    }
    // if zombie, dont add
    if (task->task_state == ZOMBIE)
       return;

    // empty list case
    if (overall_task_list == NULL)
    {
        overall_task_list = task;
    }
    else
    {
        // Move to the last & append it
        Task *temp = overall_task_list;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = task;
        task->next = NULL;
    }
}

void
wake_up_task()
{
    Task *task = overall_task_list;
    while (task != NULL)
    {
        if (task->task_state == WAIT)
        {
            task->task_state = READY;
            return;
        }
        task = task->next;
    }
    return;
}

Task *
fetch_ready_task()
{
    Task *next_task = overall_task_list;

    Task *prev = NULL;
    while (next_task != NULL)
    {
        if (next_task->task_state == READY)
        {
            next_task->task_state = RUNNING;
            break;
        }
        prev = next_task;
        next_task = next_task->next;
    }

    if (next_task == NULL)
    {
        next_task = IDLE_TASK;
    }
    else
    {
        if (prev == NULL)
        {
            overall_task_list = overall_task_list->next;
        }
        else
        {
            prev->next = next_task->next;
            next_task->next = NULL;
        }
    }

    return next_task;
}
