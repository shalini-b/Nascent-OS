#ifndef _PAGE_H
#define _PAGE_H

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

#endif