#include <sys/page.h>
#include <sys/kprintf.h>
#include <sys/types.h>

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
        kprintf("All free pages exhausted :( ");
    }

    struct page *tmp = (struct page *) get_viraddr((uint64_t)free_page_head);
    free_page_head = tmp->next;
    //FIXME: Is it correct to do this here?
    tmp->ref_count = 1;

    return tmp;
}
