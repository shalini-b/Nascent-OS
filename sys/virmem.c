#include <sys/page.h>
#include <sys/types.h>
#include <sys/virmem.h>
#include <sys/kprintf.h>
#include <sys/memset.h>

void init_mem(uint64_t *physfree, uint32_t *modulep, uint64_t *mem_end) {

   // Initiating important vars  
   pages = (struct page *)physfree;
   free_page_head = NULL;
   
   create_page_list(physfree, modulep, mem_end);
   struct page *page1 = fetch_free_page();
   
   uint64_t *pml_addr = (uint64_t *) page1; 
   create_vir_phy_mapping(pml_addr);

   // test_mapping(pml_addr);
   
   LOAD_CR3(pml_addr);
}

void test_mapping (uint64_t *pml_addr) {
   get_mapping(pml_addr, (uint64_t) KERNBASE);
   get_mapping(pml_addr, (uint64_t) KERNBASE+PHYSBASE);
   get_mapping(pml_addr, (uint64_t) KERNBASE+0xb8000);
}

void create_vir_phy_mapping(uint64_t *pml_addr) {
    uint64_t cnt = 0;
    while (cnt < page_count) {
        uint64_t viraddr = KERNBASE + cnt * PAGE_SIZE;
        // PML and PDPTE table
        uint64_t *addr = pml_addr + ((viraddr >> 39) & 0x1FF);
        uint64_t *pdpte = (uint64_t *) create_dir_table(viraddr, addr);
        // PDT table
        addr = pdpte + ((viraddr >> 30) & 0x1FF);
        uint64_t *pde = (uint64_t *) create_dir_table(viraddr, addr);
        // PTE table
        uint64_t *res = create_pde(viraddr, pde);

        // FIXME: check for value in res to be empty or not
        *res = (0x00 + cnt * PAGE_SIZE) | 3;
      
        cnt++;
    }
}

void get_mapping(uint64_t *pml_addr, uint64_t viraddr) {
        // First PML table
        uint64_t *addr = pml_addr + ((viraddr >> 39) & 0x1FF);
        uint64_t *pdpte = (uint64_t *) create_dir_table(viraddr, addr);
        // PDPTE table
        addr = pdpte + ((viraddr >> 30) & 0x1FF);
        uint64_t *pde = (uint64_t *) create_dir_table(viraddr, addr);

        uint64_t *res = create_pde(viraddr, pde);

        kprintf("Mapping for viraddr %p is %p", viraddr, *res);
}


uint64_t create_dir_table(uint64_t viraddr, uint64_t *addr) {

  uint64_t next_addr_value = (uint64_t) *addr;
  if (!(next_addr_value & 1)) {
      uint64_t newPhyAddr = (uint64_t) fetch_free_page();
      // FIXME: USR permission required?
      *(addr) = newPhyAddr | 3;
      next_addr_value = (uint64_t) *(addr);
  }
  return (uint64_t) ScaleDown((uint64_t *) next_addr_value);
}

uint64_t *create_pde(uint64_t viraddr, uint64_t *pde_addr) {

  uint64_t *addr = pde_addr + ((viraddr >> 21) & 0x1FF);
  uint64_t addr_val = (uint64_t) *(addr);
  uint64_t page_offset = ((viraddr >> 12) & 0x1FF);

  if (!(addr_val & 1)) {
      uint64_t newPhyAddr = (uint64_t) fetch_free_page();
      *(addr) = newPhyAddr | 3;
      addr_val = (uint64_t) *(addr);
  }
  uint64_t *pte = ScaleDown((uint64_t *) addr_val);
  return pte + page_offset;
}

void create_page_list(uint64_t *physfree, uint32_t *modulep, uint64_t *mem_end) {

    struct smap_t
    {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    page_count = (uint64_t) mem_end / PAGE_SIZE;
   // Keeping buffer for page tables and pages array 
   physfree = ScaleUp((uint64_t *) ((uint64_t) physfree + sizeof(struct page) * page_count));

   // Traversing pages to assign free pages & page list array proper values
   memset((void* )physfree, 0, 5*PAGE_SIZE);
   for (int page_num = 0; page_num < page_count; page_num++){
      uint64_t page_start = page_num * PAGE_SIZE;
      uint64_t page_end = page_start + PAGE_SIZE - 1;

      // FIXME: add rest of the locations to free list once paging is done
      if (page_start < (uint64_t) physfree || page_end < (uint64_t) physfree) {
          pages[page_num].ref_count = 1;
      }
      else {
           int page_marked = 0;
          for (smap = (struct smap_t *) (modulep + 2); smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4); ++smap)
          {
              if (smap->type == 1 && smap->length != 0)
              {
                  if ((page_start < smap->base + smap->length && page_start  > smap->base) && 
                         (page_end < smap->base + smap->length && page_end  > smap->base)) 
                  {
                      page_marked = 1;
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
          if (page_marked == 0) {
              pages[page_num].ref_count = 1;
        }
      } 
   } 
}


uint64_t *ScaleDown(uint64_t *phyaddr) {
   return (uint64_t *) ((uint64_t) phyaddr & 0xFFF000);
}

uint64_t *ScaleUp(uint64_t *phyaddr) {
   if(((uint64_t)phyaddr % PAGE_SIZE) != 0) {
      phyaddr = (uint64_t *) ((uint64_t) phyaddr | 0xFFF) + 1;
   }
   return phyaddr;
}

struct page* fetch_free_page() {
    // FIXME: handle no free page
    if ((free_page_head == NULL) || (free_page_head == free_page_end)) {
       // return NULL;
      kprintf("Out of free pages!!!");
    }

    struct page* tmp = free_page_head;
    struct page* free_pg = (struct page *) getPA(tmp);
    //FIXME: Is it correct to do this here?
    tmp->ref_count = 1;

    free_page_head = free_page_head->next;

    return free_pg;
}

