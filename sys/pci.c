#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/defs.h>
#include <sys/ahci.h>

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__ ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void outl( uint16_t port, uint32_t val )
{
   __asm__ __volatile__("outl %0, %1" :: "a"(val), "Nd"(port) );
}

uint16_t pciConfigReadWord (uint8_t bus, uint8_t device,
                             uint8_t func, uint8_t offset)
 {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(0xCF8, address);
    tmp = (uint16_t)((inl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
 }


uint32_t pciConfigReadLong (uint8_t bus, uint8_t device,
                             uint8_t func, uint8_t offset)
 {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(0xCF8, address);
    tmp = (uint32_t)(inl (0xCFC));
    return (tmp);
 }


int pciCheckForAHCI(uint8_t bus, uint8_t device)
 {
     uint16_t resp, vendor;
     vendor = pciConfigReadWord(bus,device,0,0);
     if (vendor == 0xFFFF){
      return 0;
     }
    
     resp = pciConfigReadWord(bus,device,0,10);
    if (resp == 0x0106) {
        return 1;
    }
     else {
	return 0;
    }
 }


void probe_port(hba_mem_t *abar)
{
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				kprintf("SATA drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				kprintf("SATAPI drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				kprintf("SEMB drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM)
			{
				kprintf("PM drive found at port %d\n", i);
			}
			else
			{
				kprintf("No drive found at port %d\n", i);
			}
		}
 
		pi >>= 1;
		i ++;
	}
}

static int check_type(hba_port_t *port)
{
	uint32_t ssts = port->ssts;
	kprintf("Drive status: %p", ssts); 
 
	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}


void checkAllBuses() {
     uint8_t bus;
     uint8_t device;
     int found;
     hba_mem_t *abar;
     uint32_t bar5;
 
     for(bus = 0; bus < 255; bus++) {
         for(device = 0; device < 32; device++) {
             found=0;
             found = pciCheckForAHCI(bus, device);
             if (found == 1){
                 kprintf("Found AHCI");
                 // bar4 =  pciConfigReadLong(bus, device, 0, 20);
                 // kprintf("BAR4 %p", bar4);
	         bar5 = pciConfigReadLong(bus, device, 0, 24);
                 kprintf("BAR5 %p", bar5);
		 outl(0xCF8, bar5);
		 abar = (hba_mem_t *)(inl (0xCFC));
		 probe_port(abar); 
             }
         }
     }
}
