#include <sys/page.h>
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
