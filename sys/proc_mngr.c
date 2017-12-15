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

struct vma *free_vma_head = NULL;
struct vma vma_arr[NUM_VMA];

extern uint64_t *kpml_addr;

Task pcb_arr[NUM_PCB];
Task *free_pcb_head = NULL;

// FIXME: call this somewhere in main??
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
    // FIXME: is it enough??
    uint64_t *pml_addr = (uint64_t *) page_alloc();
    kprintf("In create proc PML = %p\n", pml_addr);
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
    // proc_mm->pml4 = create_new_pml_table();
    proc_mm->vma_head = NULL;
    proc_mm->count = 0;

    // Initiating process vars
    free_pcb->task_mm = proc_mm;
    // CAUTION: check if it works
    free_pcb->regs.cr3 = ((uint64_t) create_new_pml_table()) - KERNBASE;
    free_pcb->regs.krsp = (uint64_t)(&free_pcb->kstack[509]);
    free_pcb->next = NULL;
    free_pcb->prev = NULL;
    free_pcb->parent_task = NULL;
    //free_pcb->childnode = NULL;
    //free_pcb->sib = NULL;
    //free_pcb->num_child = 0;

    // FIXME: check this value for errors
    free_pcb->pid = ((uint64_t) free_pcb - (uint64_t) & pcb_arr[0]) / sizeof(Task);
    kprintf("PID of new process: %d\n", free_pcb->pid);
    free_pcb->ppid = 0;
    free_pcb->task_state = READY;

    // FIXME: do we need to memset before reuse of PCB?
    // FIXME: memset regs values?
    // memset((void*)free_pcb->kstack, 0, KSTACK_SIZE);
    // memset((void*)free_pcb->fd_array, 0, MAX_FDS * sizeof(fd));

    // initiate 0, 1, 2 fd for each pcb
    for (int i = 0; i < 3; i++)
    {
        free_pcb->fd_array->fdtype = STD_FD;
        free_pcb->fd_array->alloted = 1;
        free_pcb->fd_array->file_ptr = NULL;
        free_pcb->fd_array->file_sz = 0;
        free_pcb->fd_array->num_bytes_read = 0;
        free_pcb->fd_array->is_dir = 0;
        free_pcb->fd_array->last_matched_header = NULL;
    }

    return free_pcb;
}

// ********** Process scheduling helper methods ********

void
idle_process()
{
//    kprintf("entered the idle process");
    while (1)
    {
        __asm__ __volatile__("sti;");
        __asm__ __volatile__("hlt;");
        __asm__ __volatile__("cli;");
        schedule();
    }
}

void
create_idle_process( )
{
    // CAUTION - This is a kernel process
    IDLE_TASK = fetch_free_pcb();
    IDLE_TASK->task_state = READY;
    IDLE_TASK->regs.rip =(uint64_t)&idle_process;
//    IDLE_TASK->regs.rsp = ((uint64_t) page_alloc()) + (0x1000);
    str_copy("Idle", IDLE_TASK->filename);
    add_to_task_list(IDLE_TASK);
    // FIXME: add process to list here
}

void
append_in_free_list(Task *task)
{
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
    // If idle process, dont add
    if (task->task_state == IDLE)
    {
        return;
    }
    // FIXME: Clean and put it at the end??
    if (task->task_state == EXIT)
    {
        append_in_free_list(task);
        return;
    }
    // Mark a running task as ready if it was running
    if (task->task_state == RUNNING)
    {
        task->task_state = READY;
    }
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
        // FIXME: Change this back. HACK!!!
        next_task = RunningTask;
        // next_task = IDLE_TASK;
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