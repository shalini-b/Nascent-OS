#include <sys/idt.h>
#include <sys/defs.h>
#include <sys/pic.h>
#include <sys/kprintf.h>
#include <sys/page.h>

extern void sys_int();
extern void timer();
extern void keyboard();
extern void int13();
extern void pgfault();
extern void isr_handler();

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void outb( uint16_t port, uint8_t val )
{
   __asm__ __volatile__("outb %0, %1" :: "a"(val), "Nd"(port) );
}

void load_idt(void *idt_ptr)
{
__asm__ __volatile__("lidt (%0)"::"r" (idt_ptr));
}

// Define IDT
struct idtr_t {
   uint16_t limit;
   uint64_t offset;
} __attribute__((packed));

// For each entry in IDT
struct idt_row {
   uint16_t offset_low; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes
   uint16_t offset_mid; // offset bits 16..31
   uint32_t offset_high; // offset bits 32..63
   uint32_t zero1;
} __attribute__((packed));

// Create static IDT
static struct idt_row idt[256];
static struct idtr_t idtr = { sizeof(idt) - 1, (uint64_t)idt };

void _x86_64_asm_lidt(struct idtr_t *idtr);

void add_idt(uint64_t func_base, int offset, uint64_t type_attr) {
  struct idt_row *id = (struct idt_row*)&idt[offset];
  id->offset_low = (func_base) & 0xFFFF;
  id->selector = 0x08;
  id->zero = 0;
  id->type_attr = type_attr;
  id->offset_mid = ((func_base) >> 16) & 0xFFFF;
  id->offset_high = ((func_base) >> 32) & 0xFFFFFFFF;
  id->zero1 = 0;
}

void gen_int_handler() {
    kprintf("in genneric");
    kprintf("Oops! Interrupt received. O.o ");
    outb(0x20, 0x20);
}

void int_handler13() {
    kprintf("Received Interrupt number 13!! Please check..");
    kprintf("in 13");
    outb(0x20, 0x20);

}

void page_fault_handler(uint64_t num) {
    kprintf("First parameter to page fault handler %p\n", num );
    uint64_t faulting_addr = read_cr2();
    kprintf("Faulting address : CR2 value %p\n", faulting_addr);
    outb(0x20, 0x20);
}

void init_idt() {
  // Fill up IDT here
    PIC_remap();
    for (int i = 0; i < 32; i++) {
        add_idt((uint64_t) isr_handler, i, 0x8E);
    }
    // Overriding 13 & 14
    add_idt((uint64_t)int13, 13, 0x8E);
    add_idt((uint64_t)pgfault, 14, 0x8E);
   
    add_idt((uint64_t)timer, 32, 0x8E);
    add_idt((uint64_t)keyboard,33, 0x8E);

    // FIXME: Close port for slave from 40 onwards
    for (int j = 34; j < 128; j++) {
        add_idt((uint64_t) isr_handler, j, 0x8E);
    }
    add_idt((uint64_t)sys_int, 128, 0xEE);
    for (int j = 129; j < 256; j++) {
        add_idt((uint64_t) isr_handler, j, 0x8E);
    }
    // Call LIDT
    load_idt(&idtr);
}

