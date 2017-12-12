#include<sys/types.h>
#include <sys/page.h>
#include<sys/kprintf.h>
#include<sys/process.h>


void
page_fault_handler(uint64_t* num)
{
    kprintf("in 14");
    uint64_t error_code = *num;
    kprintf("Error code to page fault handler %d\n", error_code);
    uint64_t faulting_addr = read_cr2();
    kprintf("Faulting address : CR2 value %p\n", faulting_addr);
//    print_status(error_code);
//    if (check_vma(faulting_addr))
//    {
//        if(error_code==7)
//        {
//
//        }
//    }
//    else
//    {
//        kprintf("Segmentation Fault");
//    }
//    outb(0x20, 0x20);
}




//void
//print_status(uint64_t virtual_address)
//{
//    if (pf_err_code & PF_BIT_2)
//    {
//        kprintf("user(r3) |\n");
//    }
//    else
//    {
//        kprintf("kernel(r0) |\n");
//    }
//
//    if (pf_err_code & PF_BIT_1)
//    {
//        kprintf("write | ");
//    }
//    else
//    {
//        kprintf("read mode | ");
//    }
//
//
//    if (virtual_address & PF_B_0)
//    {
//        kprintf("page present");
//    }
//    else
//    {
//        kprintf("page not present");
//    }
//
//}
//
//vma *
//check_vma(uint64_t v_addr)
//{
//    vma *itr = RunningTask->task_mm->vma_head;
//    while (itr != NULL)
//    {
//        uint64_t start_addr = itr->start_addr;
//        uint64_t end_addr = itr->end_addr;
//        if (v_addr >= start_addr && v_addr <= end_addr)
//        {
//            return itr;
//        }
//        itr = itr->next;
//    }
//    return NULL;
//}
//
