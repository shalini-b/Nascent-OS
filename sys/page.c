#include <sys/page.h>
#include <sys/types.h>

uint64_t get_viraddr(uint64_t paddr) {
   return KERNBASE + paddr;
}
