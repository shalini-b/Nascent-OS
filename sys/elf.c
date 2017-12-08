#include<sys/types.h>
#include<sys/elf64.h>
#include<sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/virmem.h>
#include<sys/page.h>
#include <sys/proc_mngr.h>
#include <sys/memset.h>

// FIXME: get corresponding sys file
#include <strings.h>

extern uint64_t *kpml_addr;

int is_elf_format(Elf64_Ehdr *elf_header)
{
    if (elf_header->e_ident[0] == 127 && elf_header->e_ident[1] == 'E' && elf_header->e_ident[2] == 'L'
        && elf_header->e_ident[3] == 'F')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// FIXME: change return type
int elf_read(struct Elf64_Ehdr *elf_header, Task *new_pcb, char *filename, char *argv[])
{
    if (is_elf_format(elf_header) == 1)
    {
        //Save current pml address
        uint64_t * prev_pml_addr;
        // FIXME: check this
        uint64_t * pml_addr = (uint64_t *) (new_pcb->regs.cr3 + KERNBASE);
        // kprintf("In load elf %p", pml_addr);
        READ_CR3(prev_pml_addr);
        // Copy file name
        str_copy(filename, new_pcb->filename);

        // initiate headers
        uint64_t start_viraddr;
        Elf64_Phdr *present_program_header;
        volatile int n_ph = elf_header->e_phnum;
        Elf64_Phdr *program_header = (struct Elf64_Phdr *) ((uint64_t) elf_header + elf_header->e_phoff);
        present_program_header = program_header;
        uint64_t max_addr = 0;

        for (int program_headers_count = 0; program_headers_count < n_ph; program_headers_count++)
        {
            if (present_program_header->p_type == 1)
            {
                // decide type of VMA
                uint64_t vm_type;
                if (present_program_header->p_flags == 5) {
                    vm_type = TEXT;
                } else if (present_program_header->p_flags == 6) {
                    vm_type = DATA;
                } else {
                    vm_type = NONE;
                }

                // calculate start & end address of address space
                start_viraddr = (uint64_t) present_program_header->p_vaddr;
                uint64_t end_address = start_viraddr + (uint64_t)(present_program_header->p_memsz);
                // create new vma, assign start & end address and increase count
                struct vma* prg_vma = fetch_free_vma(start_viraddr, end_address, present_program_header->p_type, vm_type);
                new_pcb->task_mm->count++;

                // calulate end address of process address range
                if (max_addr < end_address) {
                    max_addr = end_address;
                }

                // create the page tables for binary
                for (uint64_t pres_page_base_vir = start_viraddr; pres_page_base_vir < end_address;
                     pres_page_base_vir += (4 * 1024))
                {
                    // FIXME: always memset pages given to stack & heap
                    uint64_t phys_addr = (uint64_t) kmalloc();
                    set_mapping((uint64_t)pml_addr, (uint64_t)ScaleDown((uint64_t*)pres_page_base_vir), (uint64_t)phys_addr, 7);
                    //  kprintf("virtual address of binary %p\n", pres_page_base_vir);
                }

                // Place VMA at the end of process vma list
                if (new_pcb->task_mm->vma_head == NULL) {
                    new_pcb->task_mm->vma_head = prg_vma;
                }
                else {
                    struct vma* tmp = new_pcb->task_mm->vma_head;
                    while(tmp->next != NULL) {
                        tmp = tmp->next;
                    }
                    tmp->next = prg_vma;
                }

                void *present_file_segment =
                    (void *) ((uint64_t) elf_header + (uint64_t) present_program_header->p_offset);

                // Get into the new process space
                // __asm__ __volatile__ ("movq %0, %%cr3;"::"r"(((uint64_t) kpml_addr) - KERNBASE));
                // FIXME: Remove this after page fault handler is implemented!!
                LOAD_CR3(new_pcb->regs.cr3);

                // FIXME: Do we need to do mmap for this range given to us?
                // FIXME: write mmap with start_addr, size, flags as arguments or use kmalloc?
                // mmap(start_viraddr, present_program_header->p_memsz, vm_type == TEXT ? RX_USER : RW_USER);

//                kprintf("mapped range %x - %x\n", start_viraddr,end_address);
//                kprintf("memcopy range %x - %x\n", start_viraddr,start_viraddr+present_program_header->p_filesz);

                // Copy the contents into address space
                memcopy((void *)present_file_segment, (void *) start_viraddr, (uint64_t)program_header->p_filesz);
                int bss_size = (present_program_header->p_memsz)-(program_header->p_filesz);
                memset((void*)((start_viraddr + program_header->p_filesz)), 0, bss_size);

//                kprintf("virtual address of binary %p\n", present_program_header->p_vaddr);
//                kprintf("physical address of binary %p\n", present_program_header->p_paddr);
//                kprintf("file size %d\n", present_program_header->p_filesz);
//                kprintf("mem size %d\n", present_program_header->p_memsz);
            }
            present_program_header += 1;
        }

        // Allocate Heap VMA
        struct vma* tmp = new_pcb->task_mm->vma_head;
        while(tmp->next != NULL) {
            tmp = tmp->next;
        }
        // increment the address by 10, safe side
        start_viraddr = ((((max_addr - 1) >> 12) + 10) << 12);
        uint64_t end_address = ((((max_addr - 1) >> 12) + 10) << 12);
        // create HEAP VMA and increment count
        // Assign values to start_brk in mm struct to support mmap
        tmp->next = fetch_free_vma(start_viraddr, end_address, RW, HEAP);
        new_pcb->task_mm->count++;

        // FIXME: add new process to running queue???
        return 0;
    }
    else
    {
        kprintf("Not elf file");
        return -1;
    }
}
