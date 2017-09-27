#include <sys/defs.h>
#include <sys/ahci.h>

uint32_t inl(uint16_t port);
void outl( uint16_t port, uint32_t val);
uint16_t pciConfigReadWord (uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
int pciCheckForAHCI(uint8_t bus, uint8_t device, uint8_t f);
hba_port_t* probe_port(hba_mem_t *abar);
hba_port_t* checkAllBuses();
