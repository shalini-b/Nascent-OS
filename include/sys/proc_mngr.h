#ifndef __PROCESS_MNGR_H__
#define __PROCESS_MNGR_H__

#include <sys/process.h>
#include <sys/types.h>

void initialise_vma();
struct vma * fetch_free_vma();
void create_pcb_list();
struct Task * fetch_free_pcb();
struct vma * get_free_vma();

#endif