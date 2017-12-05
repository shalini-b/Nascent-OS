#include <sys/defs.h>
#include <sys/process.h>
#include <sys/task.h>
#include <sys/kprintf.h>
#include <sys/page.h>
#include <sys/virmem.h>

struct vma * free_vma_head;
struct vma * vma_arr;
extern int num_proc;

Task * pcb_arr;
Task * free_pcb_head;

void initialise_vma() {
    for (int i = num_vmas-1; i >= 0; i--) {
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

struct vma * fetch_free_vma() {
    if (free_vma_head==NULL) {
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma * free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;
    free_vma->next = NULL;
    free_vma->prev = NULL;

    return free_vma;
}

void create_pcb_list() {
    for (int i = num_pcbs-1; i >= 0; i--) {
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

struct Task * fetch_free_pcb() {
    if (free_pcb_head==NULL) {
        kprintf("Uh Oh! No PCB's left!!");
        return NULL;
    }
    struct Task * free_pcb = free_pcb_head;
    // FIXME: initialise fd_array & regs here?
    free_pcb_head = free_pcb_head->next;
    free_pcb->next = NULL;
    free_pcb->prev = NULL;
    free_pcb->pml4 = NULL;
    free_pcb->cr3 = NULL;
    free_pcb->vma_pntr = NULL;
    free_pcb->parent_task = NULL;
    free_pcb->pid = -1;
    free_pcb->ppid = -1;
    for (int i = 0; i<MAX_FDS; i++) {
        if (i==0 | i==1 | i==2) {

        }
    }
    // FIXME: will this create a prob?
    free_pcb->is_foregrnd = 0;

    return free_pcb;
}

int fork_process(Task * parent_pcb) {
    Task * child_pcb = fetch_free_pcb();
    child_pcb->parent_task = parent_pcb;
    // FIXME: set is_foregrnd value
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
        if (parent_pcb->fd_array[i] == 0) {
            child_pcb->fd_array[i] = 0;
        }
        else {
            child_pcb->fd_array[i] = parent_pcb->fd_array[i];
            parent_pcb->fd_array[i].ref_count++;
        }
    }

    // update parent TLB with right values
    invalidate_tlb((uint64_t) parent_pcb->cr3);

    // FIXME: update stack pointers!!
    return 0;
}

int create_pid() {
    return ++num_proc;
}

//void create_process(char *path) {
//}