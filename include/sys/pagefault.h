
#ifndef __PG_FLT_H__
#define __PG_FLT_H__

#define PF_B_0 1
#define PF_B_1 2
#define PF_B_2 4

void
page_fault_handler(uint64_t num);
void
print_status(uint64_t virtual_address);
struct vma *
check_vma(uint64_t v_addr);
void
print_status(uint64_t virtual_address);
#endif
