#ifndef __PROCESS_MNGR_H__
#define __PROCESS_MNGR_H__

#include <sys/process.h>
#include <sys/types.h>

Task* overall_task_list;

// Idle task: This will run only when there is no READY task to be run
Task * IDLE_TASK;

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

#endif