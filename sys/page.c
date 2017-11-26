#include <sys/types.h>
#include <sys/page.h>
#include <sys/kprintf.h>


uint64_t get_viraddr(uint64_t paddr) {
   return KERNBASE + paddr;
}

uint64_t convertVA(struct page* page1) {
   return get_viraddr(getPA(page1));
}

uint64_t getPA(struct page* page1) {
  return (uint64_t) ((((uint64_t) page1 - (uint64_t) pages) / sizeof(struct page)) * PAGE_SIZE);
}

uint64_t get_phyaddr(uint64_t vir_addr) {
    return vir_addr - KERNBASE;
}

struct page *page_alloc() {
    // FIXME: handle no free page
    if ((free_page_head == NULL) || (free_page_head == free_page_end)) {
        // return NULL;
        kprintf("Out of free pages!!!");
    }
    // Increment free_page_head and increase ref_count
    struct page* tmp = (struct page*) get_viraddr((uint64_t)free_page_head);
    //FIXME: Is it correct to do this here?
    tmp->ref_count = 1;
    // return viraddr of page
    struct page* free_pg = (struct page *) convertVA(free_page_head);
    free_page_head = tmp->next;
    return free_pg;
}

uint64_t *kmalloc(int size) {
    // FIXME: handle no free page
    if ((free_page_head == NULL) || (free_page_head == free_page_end)) {
        // return NULL;
        kprintf("Out of free pages!!!");
    }
    // Increment free_page_head and increase ref_count
    struct page* tmp = (struct page*) get_viraddr((uint64_t)free_page_head);
    //FIXME: Is it correct to do this here?
    tmp->ref_count = 1;
    // return phyaddr of page
    uint64_t *free_pg = (uint64_t *) getPA(free_page_head);
    free_page_head = tmp->next;
    return free_pg;
}