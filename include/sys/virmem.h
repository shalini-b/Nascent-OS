#ifndef _VIRMEM_H
#define _VIRMEM_H

#include <sys/types.h>
#include <sys/page.h>

void init_mem(uint64_t *physfree, uint32_t *modulep, uint64_t *mem_end);
void create_vir_phy_mapping(uint64_t *pml_addr);
uint64_t create_dir_table(uint64_t vir_addr);
uint64_t *create_pde(uint64_t viraddr, uint64_t *pde_addr);
void create_page_list(uint64_t *physfree, uint32_t *modulep, uint64_t *mem_end);
struct page* fetch_free_page();
uint64_t get_mapping(uint64_t pml_addr, uint64_t viraddr);
void set_mapping(uint64_t pml4_addr, uint64_t viraddr, uint64_t phyaddr, uint64_t flags);
int get_pte_entry(uint64_t pml_addr, uint64_t viraddr, uint64_t *phy_addr);


#endif
