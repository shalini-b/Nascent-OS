#ifndef __PROCESS_MNGR_H__
#define __PROCESS_MNGR_H__

#include <sys/process.h>
#include <sys/types.h>

Task* overall_task_list;

extern struct vma *free_vma_head;
struct vma vma_arr[NUM_VMA];

Task pcb_arr[NUM_PCB];
extern Task *free_pcb_head;

// Idle task: This will run only when there is no READY task to be run
Task * IDLE_TASK;
Task * INIT_TASK;

void initialise_vma();
struct vma * fetch_free_vma();
void create_pcb_list();
struct Task * fetch_free_pcb();
struct vma * get_free_vma();
void append_in_free_list(Task * task);
void create_idle_process();
void idle_process();
void add_to_task_list(Task* task);
Task * fetch_ready_task();
void wake_up_task();
void idle_process();
uint64_t wait_for_child();
void init_process();
void clean_child_pcb();
void create_init_process();
void clear_pcb(Task *cur_task);

#endif
