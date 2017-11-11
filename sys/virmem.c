#include <sys/page.h>
#include <sys/types.h>
#include <sys/virmem.h>
#include <sys/kprintf.h>

void init_mem(uint64_t *physfree, uint32_t *modulep, uint64_t *mem_end) {

   // Initiating important vars  
   // FIXME: convert to PA??
   pages = (struct page *)physfree;
   free_page_head = NULL;
   
   create_page_list(physfree, modulep, mem_end);
   struct page *page1 = fetch_free_page();
   
   // FIXME: converting to PA to get pml4
   uint64_t *pml_addr = (uint64_t *) KERNBASE + (page1 - pages) * PAGE_SIZE; 
   create_vir_phy_mapping(pml_addr);

   LOAD_CR3(pml_addr - KERNBASE);
}


void create_vir_phy_mapping(uint64_t *pml_addr) {
    uint64_t cnt = 0;
    // FIXME: should we map only till physfree? 
    // if yes, then physfree might not be aligned to a 4k address 
    while (cnt < page_count) {
        uint64_t viraddr = KERNBASE + cnt * PAGE_SIZE;
        // First PML table
        uint64_t *addr = pml_addr + ((viraddr >> 39) & 0x1FF);
        uint64_t pdpte_addr = create_dir_table(viraddr, addr);
        // PDPTE table
        // Converting to vir addr
        // FIXME: align to 4k?
        uint64_t *pdpte = (uint64_t *) pdpte_addr + KERNBASE;
        addr = pdpte + ((viraddr >> 30) & 0x1FF);
        uint64_t pde_addr = create_dir_table(viraddr, addr);

        // FIXME: align to 4k?
        uint64_t *pde = (uint64_t *) pde_addr + KERNBASE;
        uint64_t *res = create_pde(viraddr, pde);

        // FIXME: check for value in res to be empty or not
        *res = (0x00 + cnt * PAGE_SIZE) | 3;

        cnt++;
    }
}

uint64_t create_dir_table(uint64_t viraddr, uint64_t *addr) {

  uint64_t next_addr_value = (uint64_t) *addr;
  if (!(next_addr_value & 1)) {
      // FIXME: handle no free page
      struct page* new = fetch_free_page();
      uint64_t newPhyAddr = (uint64_t) ((new - pages) * PAGE_SIZE);
      // FIXME: USR permission required?
      *(addr) = newPhyAddr | 7;
      next_addr_value = (uint64_t) *(addr);
  }
  return next_addr_value;
}

uint64_t *create_pde(uint64_t viraddr, uint64_t *pde_addr) {

  uint64_t *addr = pde_addr + ((viraddr >> 21) & 0x1FF);
  uint64_t addr_val = (uint64_t) *(addr);
  uint64_t page_offset = ((viraddr >> 12) & 0x1FF);

  if (!(addr_val & 1)) {
      // FIXME: handle no free page
      struct page* new = fetch_free_page();
      uint64_t newPhyAddr = (uint64_t) ((new - pages) * PAGE_SIZE);
      *(addr) = newPhyAddr | 7;
      addr_val = (uint64_t) *(addr);
  }
  // FIXME: align to 4k?
  uint64_t *pte =(uint64_t *) addr_val + KERNBASE;
  return &pte[page_offset];
}

void create_page_list(uint64_t *physfree, uint32_t *modulep, uint64_t *mem_end) {

    struct smap_t
    {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    kprintf("Memory end pointer %p \n", mem_end);
    // FIXME: make this dynamic
    page_count = (uint64_t) mem_end / PAGE_SIZE;
    // page_count = 32735;
    // FIXME: align this to nearest 4k page
    physfree = physfree + sizeof(struct page) * page_count + 5 * PAGE_SIZE; // Keeping buffer for page tables and pages array
   // Traversing pages to assign free pages & page list array proper values
   for (int page_num = 0; page_num < page_count; page_num++){
      uint64_t page_start = page_num * PAGE_SIZE;
      uint64_t page_end = page_start + PAGE_SIZE - 1;

      // FIXME: add rest of the locations to free list once paging is done
      if (page_start <= (uint64_t) physfree || page_end <= (uint64_t) physfree) { // FIXME: should it be page_end < physfree?
          pages[page_num].ref_count = 1;
      }
      else {
           int page_marked = 0;
          // FIXME: take care of holes in this space
          for (smap = (struct smap_t *) (modulep + 2); smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4); ++smap)
          {
              if (smap->type == 1 && smap->length != 0)
              {
                  if ((page_start < smap->base + smap->length && page_start  > smap->base) || 
                         (page_end < smap->base + smap->length && page_end  > smap->base)) 
                  {
                      page_marked = 1;
                      pages[page_num].ref_count = 1;
                  }
                  
              }
          }
          if (page_marked == 0) {
              if (free_page_head == NULL) {
                 pages[page_num].ref_count = 0;
                 free_page_head = &pages[page_num];
                 free_page_head->next = NULL;
                 free_page_end = free_page_head;

              }
              else {
                 pages[page_num].ref_count = 0;
                 free_page_end->next = &pages[page_num];
                 free_page_end = &pages[page_num];
                 free_page_end->next = NULL;
              }
        }
      } 
   } 
}

struct page* fetch_free_page() {
    // FIXME: handle no free page
    if ((free_page_head == NULL) || (free_page_head == free_page_end)) {
       // return NULL;
      kprintf("Out of free pages!!!");
    }

    struct page* tmp = free_page_head;
    free_page_head = free_page_head->next;
    //FIXME: Is it correct to do this here?
    tmp->ref_count = 1;

    return tmp;
}


