#define PF_B_0 1
#define PF_B_1 2
#define PF_B_2 4

void
page_fault_handler(uint64_t* num);
void
print_status(uint64_t virtual_address);
void
check_vma(uint64_t v_addr);
