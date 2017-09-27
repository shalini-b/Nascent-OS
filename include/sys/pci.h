#include <sys/defs.h>
#include <sys/ahci.h>

extern hba_port_t *port_ptr;

uint32_t inl(uint16_t port);
void outl( uint16_t port, uint32_t val);
uint16_t pciConfigReadWord (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
int pciCheckForAHCI(uint8_t bus, uint8_t device, uint8_t f);
void probe_port(hba_mem_t *abar);
void checkAllBuses();
