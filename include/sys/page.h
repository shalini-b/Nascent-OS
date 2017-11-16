#ifndef _PAGE_H
#define _PAGE_H

#include <sys/types.h>
#define PAGE_SIZE 4096
#define TBL_ENTRIES 512

#define PHYSBASE 0x200000
#define PHYSFREE 0x215000
#define PAGE_SIZE_BIN 0x1000
#define KERNBASE 0xffffffff80000000

#define LOAD_CR3(val) __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(val));

struct page {
   struct page *next;
   int ref_count;  
};

struct page* free_page_end;
struct page* free_page_head;
struct page* pages;

int page_count;

// void memset(char *initial_mem, int length,int value_memset);
uint64_t get_viraddr(uint64_t paddr);
uint64_t convertVA(struct page* page1);
uint64_t getPA(struct page* page1);
uint64_t get_phyaddr(uint64_t vir_addr);
uint64_t *ScaleDown(uint64_t *phyaddr);
uint64_t *ScaleUp(uint64_t *phyaddr);
void test_mapping(uint64_t *pml_addr);

#endif
