#include <sys/defs.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/page.h>
#include <sys/virmem.h>

struct vma * free_vma_head = NULL;
struct vma vma_arr[NUM_VMA];
extern int num_proc;
extern uint64_t *kpml_addr;
Task pcb_arr[NUM_PCB];
Task * free_pcb_head = NULL;

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

struct vma * fetch_free_vma(uint64_t start_addr, uint64_t end_addr, uint64_t vm_flags, VMA_TYPES vmtype) {
    if (free_vma_head==NULL) {
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma * free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;
    free_vma->next = NULL;
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

/*int get_free_fd(void *file_ptr)
{
    for (int i = 0; i < MAX_FDS; i++)
    {
        if (runningTask->fd_array[i].alloted == 0)
        {
            runningTask->fd_array[i].alloted = 1;
            runningTask->fd_array[i].file_ptr = file_ptr;
            runningTask->fd_array[i].last_matched_header = file_ptr;
            return i;
        }
    }
    return -1;
}

void initialise_fds() {
    for (int i = 0; i < MAX_FDS; i++) {
        runningTask->fd_array[i].alloted = 0;
        runningTask->fd_array[i].file_sz = 0;
        runningTask->fd_array[i].num_bytes_read = 0;
        runningTask->fd_array[i].is_dir = 0;
        runningTask->fd_array[i].last_matched_header = 0;
    }
}*/

uint64_t create_new_pml_table() {
    // FIXME: complete this!!
    pml_addr = page_alloc();
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
    struct mm * proc_mm = (struct mm_struct *) kmalloc(sizeof(struct mm_struct));
    // CAUTION: PML address is a virtual address
    proc_mm->pml4 = create_new_pml_table();
    proc_mm->vma_head = NULL;
    proc_mm->count = NULL;
    proc_mm->total_vma_size = 0;

    // Initiating process vars
    free_pcb->task_mm = proc_mm;
    free_pcb->next = NULL;
    free_pcb->prev = NULL;
    free_pcb->cr3 = NULL;
    free_pcb->parent_task = NULL;
    free_pcb->childnode = NULL;
    free_pcb->sib = NULL;
    free_pcb->num_child = 0;
    free_pcb->pid = num_proc++;
    free_pcb->ppid = 0;
    free_pcb->task_state = READY;
    memset((void*)free_pcb->kstack, 0, KSTACK_SIZE);
    memset((void*)free_pcb->fd_array, 0, MAX_FDS * sizeof(fd));

    return free_pcb;
}

int fork_process(Task * parent_pcb) {
    Task * child_pcb = fetch_free_pcb();
    child_pcb->parent_task = parent_pcb;
    child_pcb->ppid = parent_pcb->pid;
    // FIXME: checkpoint
    str_copy(child_pcb->filename, parent_pcb->filename);

    // copying file desc
    for (int i = 0; i < MAX_FDS; i++) {
        // FIXME: default value??
        if (parent_pcb->fd_array[i] != NULL) {
            // FIXME: check if copy works properly
            child_pcb->fd_array[i] = parent_pcb->fd_array[i];
            // parent_pcb->fd_array[i].alloted++;
        }
    }

    struct vma * parent_vmas = parent_pcb->task_mm->vma_head;
    struct vma * child_vmas = NULL;
    uint64_t ppml4 = parent_pcb->task_mm->pml4;
    uint64_t cpml4 = child_pcb->task_mm->pml4;
    memcopy((void*)parent_pcb->task_mm, (void*)child_pcb->task_mm, sizeof(struct mm_struct));
    child_pcb->task_mm->pml4 = cpml4;
    child_pcb->task_mm->vma_head = NULL;

    if (parent_pcb->childnode) {
        child_pcb->sib = parent_pcb->childnode;
    }
    parent_pcb->childnode = child_pcb;
    parent_pcb->num_child++;

    parent_vma = parent_pcb->task_mm->vma_head;
    while (parent_vma) {
        uint64_t start = parent_vma->start_addr;
        uint64_t end = parent_vma->end_addr;

        if (child_pcb->task_mm->vma_head == NULL) {
            child_pcb->task_mm->vma_head = fetch_free_vma(parent_vma->start_addr,
                                                          parent_vma->end_addr,
                                                          parent_vma->vm_flags,
                                                          parent_vma->vmtype);
            child_vmas = child_pcb->task_mm->vma_head;
        }
        else {
            child_vmas->next = fetch_free_vma(parent_vma->start_addr,
                                              parent_vma->end_addr,
                                              parent_vma->vm_flags,
                                              parent_vma->vmtype);
            child_vmas = child_vmas->next;
        }
    }
    // TODO!!!!





    /*
    child_pcb->parent_task = parent_pcb;
    child_pcb->regs = parent_pcb->regs;
    child_pcb->pid = create_pid();
    child_pcb->ppid = parent_pcb->pid;
    // FIXME: copy filename and time it is up
    // FIXME: set child's cwd
    // copying page tables
    uint64_t * pml_addr = (uint64_t *) page_alloc();

    //point kernel page to parent's kernel page
    uint64_t kpml_offset = ((KERNBASE >> 39) & 0x1FF);
    child_pcb->pml4[kpml_offset] = parent_pcb->pml4[kpml_offset];

    // copy VMA's
    struct vma* prev_vma;
    struct vma * prnt_vma = parent_pcb->vma_pntr;
    while (prnt_vma) {
        //create new vma
        struct vma * child_vma = fetch_free_vma();
        //first case
        if (prnt_vma == parent_pcb->vma_pntr) {
            child_pcb->vma_pntr = child_vma;
            *child_vma = *prnt_vma;
            prev_vma = child_vma;
        }
        else {
            prev_vma->next = child_vma;
            child_vma->prev = prev_vma;
            *child_vma = *prnt_vma;
            prev_vma = child_vma;
        }

        // copy page table entries
        // FIXME: Caution - these may not be required
        uint64_t end = (uint64_t) ScaleUp((uint64_t *) prnt_vma->end_addr);
        uint64_t start = (uint64_t) ScaleDown((uint64_t *) prnt_vma->start_addr);
        int cnt = (end - start)/PAGE_SIZE;

        for (int i = 0; i < cnt; i++) {
            // what viraddr to use?
            uint64_t * phyaddr = get_mapping(pml_addr, viraddr);

        }

        prnt_vma = prnt_vma->next;
    }

    // copying file desc
    for (int i = 0; i < MAX_FDS; i++) {
        // FIXME: default value??
        if (parent_pcb->fd_array[i] != NULL) {
            child_pcb->fd_array[i] = parent_pcb->fd_array[i];
            parent_pcb->fd_array[i].ref_count++;
        }
    }

    // update parent TLB with right values
    invalidate_tlb((uint64_t) parent_pcb->cr3);

    // FIXME: update stack pointers!!
    return 0; */
}


//void create_process(char *path) {
//}