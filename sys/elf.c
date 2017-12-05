#include<sys/types.h>
#include<sys/elf64.h>
#include<sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/virmem.h>
#include<sys/page.h>
#include <sys/process.h>
#include <sys/memset.h>
extern uint64_t *kpml_addr;
static char args[10][100];

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
void elf_read(struct Elf64_Ehdr *elf_header, Task *new_pcb, char *filename, char *argv[])
{
    if (is_elf_format(elf_header) == 1)
    {
        //kernel mapping
        uint64_t * prev_pml_addr;
        uint64_t * pml_addr = new_pcb->task_mm->pml4;
        READ_CR3(prev_pml_addr);
        strcpy(filename, new_pcb->filename);

        int n_ph = elf_header->e_phnum;
        uint64_t start_viraddr;
        Elf64_Phdr *program_header = (struct Elf64_Phdr *) ((uint64_t) elf_header + elf_header->e_phoff);
        Elf64_Phdr *present_program_header;
        present_program_header = program_header;
        uint64_t max_val = 0;
        for (int program_headers_count = 0; program_headers_count < n_ph; program_headers_count++)
        {
            if (present_program_header->p_type == 1)
            {
                uint64_t vm_type;
                if (present_program_header->p_flags == 5) {
                    vm_type = TEXT;
                } else if (present_program_header->p_flags == 6) {
                    vm_type = DATA;
                } else {
                    vm_type = NONE;
                }

                start_viraddr = (uint64_t) present_program_header->p_vaddr;
                uint64_t end_address = start_viraddr + (uint64_t)(present_program_header->p_memsz);
                struct vma* prg_vma = fetch_free_vma(start_viraddr, end_address, present_program_header->p_type, vm_type);
                // new_pcb->task_mm->total_vma_size = (uint64_t)(present_program_header->p_memsz);
                new_pcb->task_mm->count++;
                if (max_val < end_address) {
                    max_val = end_address;
                }

                for (uint64_t pres_page_base_vir = start_viraddr; pres_page_base_vir < end_address;
                     pres_page_base_vir += (4 * 1024))
                {
                    // FIXME: Fix kmalloc!!
                    uint64_t phys_addr = (uint64_t) kmalloc(1);
                    set_mapping((uint64_t)pml_addr, (uint64_t)ScaleDown((uint64_t*)pres_page_base_vir), (uint64_t)phys_addr);
//                    kprintf("virtual address of binary %p\n", pres_page_base_vir);
                }
                void *present_file_segment =
                    (void *) ((uint64_t) elf_header + (uint64_t) present_program_header->p_offset);

                LOAD_CR3(new_pcb->task_mm->pml4);
                // FIXME: mmap??
                mmap(start_viraddr, present_program_header->p_memsz, vm_type == TEXT ? RX_USER : RW_USER);

                if (new_pcb->task_mm->vma_head == NULL) {
                    new_pcb->task_mm->vma_head = node;
                } else {
                    struct vma* tmp = new_pcb->task_mm->vma_head;
                    while(tmp->next != NULL) {
                        tmp = tmp->next;
                    }
                    tmp->next = prg_vma;
                }
//                kprintf("mapped range %x - %x\n", start_viraddr,end_address);
//                kprintf("memcopy range %x-%x\n", start_viraddr,start_viraddr+present_program_header->p_filesz);
                memcopy((void *)present_file_segment, (void *) start_viraddr, (uint64_t)program_header->p_filesz);
                int bss_size = (present_program_header->p_memsz)-(program_header->p_filesz);
                memset((void*)((start_viraddr+program_header->p_filesz)), 0, bss_size);
//                __asm__ __volatile__ ("movq %0, %%cr3;"::"r"(((uint64_t) kpml_addr) - KERNBASE));
//                kprintf("virtual address of binary %p\n", present_program_header->p_vaddr);
//                kprintf("physical address of binary %p\n", present_program_header->p_paddr);
//                kprintf("file size %d\n", present_program_header->p_filesz);
//                kprintf("mem size %d\n", present_program_header->p_memsz);
                LOAD_CR3(prev_pml_addr);
            }
            present_program_header +=1;
            //((uint64_t) present_program_header + elf_header->e_phentsize);
        }

        // Allocate Heap VMA
        struct vma* tmp = new_pcb->task_mm->vma_head;
        while(tmp->next != NULL) {
            tmp = tmp->next;
        }
        start_viraddr = ((((max_val - 1) >> 12) + 1) << 12);
        end_address = ((((max_val - 1) >> 12) + 1) << 12);
        tmp->next = fetch_free_vma(start_viraddr, end_address, RW, HEAP);
        new_pcb->task_mm->count++;

        // Allocate Stack VMA
        struct vma* tmp = new_pcb->task_mm->vma_head;
        while(tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = fetch_free_vma((uint64_t) USTACK, (uint64_t) (USTACK - USTACK_SIZE), RW, STACK);
        LOAD_CR3(new_pcb->task_mm->pml4);
        mmap(end_address-PAGE_SIZE, PAGE_SIZE, RW_USER);
        LOAD_CR3(prev_pml_addr);
        new_pcb->task_mm->count++;
        new_pcb->task_mm->begin_stack = end_address - 0x8;

        int argc = 1;
        str_copy(filename, args[0]);
        if (argv) {
            while (argv[argc-1]) {
                str_copy(argv[argc-1], args[argc]);
                argc++;
            }
        }
        copy_arg_to_stack(new_pcb, argc);
        // FIXME: continue
    }
    else
    {
        kprintf("not elf file");
    }
}

void copy_arg_to_stack(Task *prog_pcb, int argc)
{
    uint64_t cur_pml_addr, *user_stack, *argv[10];

    READ_CR3(cur_pml_addr);
    LOAD_CR3(prog_pcb->task_mm->pml4);

    user_stack = (uint64_t *) prog_pcb->task_mm->begin_stack;
    // Store the argument values
    for (int i = argc-1; i >= 0; i--) {
        int arg_len = len(args[i]) + 1;
        user_stack = (uint64_t *)((void *) user_stack - arg_len);
        memcopy(args[i], (char*)user_stack, arg_len);
        argv[i] = user_stack;
    }
    // Store the argument pointers
    for (int i = argc-1; i >= 0; i--) {
        user_stack--;
        *user_stack = (uint64_t) argv[i];
    }
    // Store the arg count
    user_stack--;
    *user_stack = (uint64_t) argc;

    // Reset stack top below arguments
    prog_pcb->task_mm->begin_stack = (uint64_t)user_stack;

    LOAD_CR3(cur_pml_addr);
}
