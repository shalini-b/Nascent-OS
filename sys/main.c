#include <sys/defs.h>
#include <sys/gdt.h>
#include <sys/kprintf.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/idt.h>
#include <sys/pci.h>
#include <sys/virmem.h>
#include <sys/page.h>
#include <sys/test.h>
#include <sys/proc_mngr.h>
#include <sys/process.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t *loader_stack;
extern char kernmem, physbase;
//int ring_0_3_switch();
extern uint64_t *kpml_addr;
int is_first_proc = 1;

void
start(uint32_t *modulep, void *physbase, void *physfree)
{
    struct smap_t
    {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;

    uint64_t *mem_end = NULL;
    while (modulep[0] != 0x9001)
        modulep += modulep[1] + 2;

    clear_screen();
    for (smap = (struct smap_t *) (modulep + 2); smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4);
         ++smap)
    {
        if (smap->type == 1 /* memory */ && smap->length != 0)
        {
            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
            mem_end = (uint64_t *)(smap->base + smap->length);
        }
    }
    // Initiating paging
    kprintf("physfree %p\n", (uint64_t) physfree);
    init_mem((uint64_t *) physfree, modulep, mem_end);
//    kprintf("Kernel PML address: %p\n", kpml_addr);

    // Initiating interrupts
    kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
    init_idt();

    // Initialise global structures
    create_pcb_list();
    initialise_vma();

    // enable interrupts
    __asm__ __volatile__ ("sti");

    // Call exec but do not clean up the previous task
    if (is_first_proc == 1) {
        // FIXME: create a dummy task first
        RunningTask = fetch_free_pcb();
        add_to_task_list(RunningTask);
        RunningTask->task_state = RUNNING;
        sys_execvpe("bin/sbush", NULL, NULL);
    }

    // init_tasks1();
    // tarfs_test();

    /* hba_port_t* port_ptr = checkAllBuses();
    intitialise(port_ptr);
    for(int write_block=0; write_block<100; write_block++)
    {
	char* rd_buf = (char*) get_viraddr(0x70000);
        char* wt_buf = (char*) get_viraddr(0x4f000);
        for(int i=0; i<4096; i++){
            wt_buf[i]=(char)write_block;
	}
        read_write(port_ptr, write_block*8, 0, 8, (uint16_t *)wt_buf, 0);
        read_write(port_ptr, write_block*8 ,0, 8, (uint16_t *)rd_buf, 1);
       //str_cmp
       for(int i=0; i<4096; i++)
        {
            if(rd_buf[i]!=wt_buf[i])
                kprintf("mismatch between value stored and read");
        }
    }*/
    while (1);
}

void
boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1, *temp2;

    for (temp2 = (char *) get_viraddr(0xb8001); temp2 < (char *) get_viraddr(0xb8000) + 160 * 25; temp2 += 2)
        *temp2 = 7 /* white */;
    __asm__ volatile (
    "cli;"
        "movq %%rsp, %0;"
        "movq %1, %%rsp;"
    :"=g"(loader_stack)
    :"r"(&initial_stack[INITIAL_STACK_SIZE])
    );
    init_gdt();
    start(
        (uint32_t * )((char *) (uint64_t) loader_stack[3] + (uint64_t) & kernmem - (uint64_t) & physbase),
        (uint64_t * ) & physbase,
        (uint64_t * )(uint64_t)
    loader_stack[4]
    );
    for (
        temp1 = "!!!!! start() returned !!!!!", temp2 = (char *) get_viraddr(0xb8000);
        *temp1;
        temp1 += 1, temp2 += 2
        )
        *temp2 = *temp1;
    while (1) __asm__ volatile ("hlt");
}
