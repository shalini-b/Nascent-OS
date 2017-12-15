#ifndef _PAGE_H
#define _PAGE_H

#include <sys/types.h>
#define PAGE_SIZE 4096
#define TBL_ENTRIES 512

#define PHYSBASE 0x200000
#define KERNBASE 0xffffffff80000000
#define USTACK   0xfffffe0000000000
#define USTACK_SIZE 0x10000

#define LOAD_CR3(val) __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(val));
#define READ_CR3(val) __asm__ __volatile__ ("movq %%cr3, %0;" : "=r"(val));

#define BIT_COW 0x100
#define UNSET_WRITE(entry) entry = entry & 0xFFFFFFFFFFFFFFFD
#define SET_COW(entry) entry = entry | BIT_COW

#define RW_KERNEL (1UL | 2UL)
#define RX_USER (1UL | 4UL)
#define RW_USER (1UL | 4UL | 2UL)

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
struct page *page_alloc();
void *kmalloc();
uint64_t read_cr2();
void invalidate_tlb(uint64_t pml4);
struct page * get_page_from_PA(uint64_t phyaddr);

#endif
