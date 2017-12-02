#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <sys/types.h>

#define num_vmas 100
#define num_pcbs 1000

struct vma {
    uint64_t start_addr;
    uint64_t end_addr;
    struct vma * next;
    struct vma * prev;
};

struct mm_struct {
    struct vma *vma_head;
    int count;
};

void initialise_vma();
struct vma * fetch_vma();
void create_pcb_list();
struct Task * fetch_free_pcb();
int fork_process();

#endif
