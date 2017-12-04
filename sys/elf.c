#include<sys/types.h>
#include<sys/elf64.h>
#include<sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/virmem.h>
#include<sys/page.h>
#include <sys/memset.h>
extern uint64_t *kpml_addr;
uint64_t *pml_addr;
int
is_elf_format(Elf64_Ehdr *elf_header)
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

void elf_read(struct Elf64_Ehdr *elf_header)
{
    //Fixme :: fill bss
    if (is_elf_format(elf_header) == 1)
    {
        int n_ph = elf_header->e_phnum;
        uint64_t present_program_segment_vaddr;
        //kernel mapping
        pml_addr = (uint64_t *) page_alloc();
        pml_addr[511] = kpml_addr[511];
        Elf64_Phdr *program_header = (struct Elf64_Phdr *) ((uint64_t) elf_header + elf_header->e_phoff);
        Elf64_Phdr *present_program_header;
        present_program_header = program_header;
        for (int program_headers_count = 0; program_headers_count < n_ph; program_headers_count++)
        {
            if (present_program_header->p_type == 1)
            {

                present_program_segment_vaddr = (uint64_t) present_program_header->p_vaddr;
                uint64_t end_address = present_program_segment_vaddr + (uint64_t)(present_program_header->p_filesz);
                for (uint64_t pres_page_base_vir = present_program_segment_vaddr; pres_page_base_vir < end_address;
                     pres_page_base_vir += (4 * 1024))
                {
                    uint64_t phys_addr = (uint64_t) kmalloc(1);
                    set_mapping((uint64_t)pml_addr, (uint64_t)ScaleDown((uint64_t*)pres_page_base_vir), (uint64_t)phys_addr);
		    kprintf("virtual address of binary %p\n", pres_page_base_vir);


                }
                void *present_file_segment =
                    (void *) ((uint64_t) elf_header + (uint64_t) present_program_header->p_offset);
                //__asm__ __volatile__("pushq $35 ;");
                __asm__ __volatile__ ("movq %0, %%cr3;"::"r"(((uint64_t) pml_addr) - KERNBASE));
                //__asm__ __volatile__("pushq $35 ;");
                kprintf("mapped range %x - %x\n", present_program_segment_vaddr,end_address);
                kprintf("memcopy range %x-%x\n", present_program_segment_vaddr,present_program_segment_vaddr+present_program_header->p_filesz);
		memcopy(present_file_segment, (void *) present_program_segment_vaddr, program_header->p_filesz);
//                __asm__ __volatile__ ("movq %0, %%cr3;"::"r"(((uint64_t) kpml_addr) - KERNBASE));
//                kprintf("virtual address of binary %p\n", present_program_header->p_vaddr);
//                kprintf("physical address of binary %p\n", present_program_header->p_paddr);
//                kprintf("file size %d\n", present_program_header->p_filesz);
//                kprintf("mem size %d\n", present_program_header->p_memsz);
            }
            present_program_header +=1;
                //((uint64_t) present_program_header + elf_header->e_phentsize);

        }

    }
    else
    {
        kprintf("not elf file");
    }


}
