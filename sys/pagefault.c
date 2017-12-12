#include<sys/types.h>
#include <sys/page.h>
#include<sys/kprintf.h>
#include<sys/process.h>
#include <sys/pagefault.h>
#include<sys/page.h>
#include <sys/memset.h>
#include<sys/virmem.h>
#define MIN(a, b)  (a<b)? a : b
#define MAX(a, b)  (a>b)? a : b
void
page_fault_handler(uint64_t error_code)
{
//    kprintf("in 14\n");
//    kprintf("Error code to page fault handler %p\n", error_code);
    uint64_t faulting_addr = read_cr2();
    kprintf("Faulting address : CR2 value %p\n", faulting_addr);
//    print_status(error_code);
    struct vma *vma_v = check_vma(faulting_addr);
    if (vma_v != NULL)
    {
//        kprintf("vma type is :: %d",vma_v->vmtype);

        //elf case lazy loading
        uint64_t vma_start = vma_v->start_addr;
        uint64_t pml_addr = (uint64_t)RunningTask->regs.cr3 + KERNBASE;
        if ((vma_v->vmtype == TEXT) || (vma_v->vmtype == DATA))
        {
//            kprintf("page fault :: in elf vma");
            uint64_t bss_addr = vma_start+vma_v->p_filesz;
            if(faulting_addr<bss_addr)
            {
                uint64_t phys_addr = (uint64_t) kmalloc();
                uint64_t s_d_a =  (uint64_t)ScaleDown((uint64_t*)faulting_addr);

                uint64_t offset = s_d_a-vma_start;

                set_mapping((uint64_t)pml_addr, s_d_a, (uint64_t)phys_addr, 7);
                //for lower segment
                if (s_d_a<vma_start &&(s_d_a+(4*1024))>bss_addr )
                {
                    memcopy((void *)(vma_v->tarfs_base), (void *) vma_start, vma_v->p_filesz);
                }
                else if(s_d_a<=vma_start)
                {
                    memcopy((void *)(vma_v->tarfs_base), (void *) vma_start, MIN(vma_v->p_filesz,4*1024-(vma_start-s_d_a)));
                }
                    //for upper segment
                else if ((s_d_a+(4*1024))>bss_addr)
                {
                    memcopy((void *)(vma_v->tarfs_base+offset), (void *) s_d_a, (bss_addr-s_d_a));
                }
                else
                {
                    memcopy((void *)(vma_v->tarfs_base+offset), (void *) s_d_a, 4*1024);
                }
//                memcopy((void *)present_file_segment, (void *) start_viraddr, (uint64_t)program_header->p_filesz);

            }
                //bss case
            else
            {
                uint64_t phys_addr = (uint64_t) kmalloc();
                uint64_t s_d_a =  (uint64_t)ScaleDown((uint64_t*)faulting_addr);
                set_mapping((uint64_t)pml_addr, s_d_a, (uint64_t)phys_addr, 7);
            }
        }

            // cow case
        else if (error_code == 7)
        {
            if (1)
            {
                kprintf("page fault :: COW");

            }
            else
            {
                kprintf("Invalid memory access");
            }

        }

            //auto growing stack
        else if (vma_v->vmtype == STACK)
        {
            kprintf("page fault :: stack");

        }

        else
        {
            kprintf("None of the cases");
        }

    }
    else
    {
        kprintf("Segmentation Fault");
        print_status(error_code);
        while (1);
    }
//    outb(0x20, 0x20);
}

void
print_status(uint64_t e_c)
{


    kprintf("in 14\n");
    kprintf("Error code to page fault handler %p\n", e_c);
    uint64_t faulting_addr = read_cr2();
    kprintf("Faulting address : CR2 value %p\n", faulting_addr);
    if (e_c & PF_B_2)
    {
        kprintf("user(r3) |");
    }
    else
    {
        kprintf("kernel(r0) |");
    }

    if (e_c & PF_B_1)
    {
        kprintf("write | ");
    }
    else
    {
        kprintf("read mode | ");
    }

    if (e_c & PF_B_0)
    {
        kprintf("page present");
    }
    else
    {
        kprintf("page not present\n");
    }

}

struct vma *
check_vma(uint64_t v_addr)
{
    struct vma *itr = RunningTask->task_mm->vma_head;
    while (itr != NULL)
    {
        uint64_t start_addr = itr->start_addr;
        uint64_t end_addr = itr->end_addr;
        if (v_addr >= start_addr && v_addr <= end_addr)
        {
            kprintf("start end range %p-%p\n",start_addr,end_addr);
            return itr;
        }
        itr = itr->next;
    }
    return NULL;
}

