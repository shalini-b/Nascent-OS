#include <sys/defs.h>
#include <sys/process.h>
#include <sys/task.h>
#include <sys/kprintf.h>

struct vma * free_vma_head;
struct vma * vma_arr;

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

struct vma * fetch_vma() {
    if (free_vma_head==NULL) {
        kprintf("Caution! Out of VMA's..");
        return NULL;
    }
    struct vma * free_vma = free_vma_head;
    free_vma_head = free_vma_head->next;

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
    // FIXME: Anything to initialise here?
    free_pcb_head = free_pcb_head->next;

    return free_pcb;
}

int fork_process() {
    return 0;
}