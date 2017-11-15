#include <sys/ahci.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/memset.h>

int read_write(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf, int rf)
{
    port->is_rwc = (uint32_t)-1;		// Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);	// Command FIS size
    cmdheader->w = ((rf==1)?0:1);		// Read from device
    cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;	// PRDT entries count

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) +
        (cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));

    // 8K bytes (16 sectors) per PRDT
    int i=0;
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8*1024;	// 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4*1024;	// 4K words
        count -= 16;	// 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba =(uint64_t) buf;
    cmdtbl->prdt_entry[i].dbc = count<<9;	// 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Command
    cmdfis->command = ((rf==1)?ATA_CMD_READ_DMA_EX:ATA_CMD_WRITE_DMA_EX);

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl>>8);
    cmdfis->lba2 = (uint8_t)(startl>>16);
    cmdfis->device = 1<<6;	// LBA mode

    cmdfis->lba3 = (uint8_t)(startl>>24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth>>8);

    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return 0;
    }

    port->ci = 1<<slot;	// Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
        {
            kprintf("Read disk error 1\n");
            return 0;
        }
    }

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)//Fixme :: find this constant
    {
        kprintf("Read disk error 2\n");
        return 0;
    }

    return 1;
}
// Start command engine
void start_cmd(hba_port_t *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(hba_port_t *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
}
// Find a free command list slot
int find_cmdslot(hba_port_t *port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact |port->ci);
    for (int i=0; i<32; i++)
    {
        if ((slots&1) == 0)
            return i;
        slots >>= 1;
    }
    kprintf("Cannot find free command list entry\n");
    return -1;
}
void intitialise(hba_port_t *port)
{
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}


//void *memset(void *s, int c, size_t n)
//{
//    unsigned char* p=s;
//    while(n--)
//        *p++ = (unsigned char)c;
//    return s;
//}

//void  *memset(void *string_to_memset, int char_to_memset_with, int length_to_memset)
//{
//    unsigned char *temp_typecasted_string = string_to_memset;
//    for(int str_itr = 0;str_itr<length_to_memset;str_itr++)
//    {
//        temp_typecasted_string[str_itr] = char_to_memset_with;
//    }
//    return(string_to_memset);
//}
