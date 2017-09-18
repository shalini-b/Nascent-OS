#include <sys/idt.h>
#include <sys/defs.h>

void timer();
void  keyboard();
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

void load_idt(void *ptr){
__asm__ __volatile__("lidt (%0)"::"r" (ptr));
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

void add_idt(uint64_t func_base, int offset) {
  struct idt_row *id = (struct idt_row*)&idt[offset];
  id->offset_low = (func_base) & 0xFFFF;
  id->selector = 0x08;
  id->zero = 0;
  id->type_attr = 0x8E;
  id->offset_mid = ((func_base) >> 16) & 0xFFFF;
  id->offset_high = ((func_base) >> 32) & 0xFFFFFFFF;
  id->zero1 = 0;
}

void init_idt() {
  // Fill up IDT here
//  add_idt((uint64_t)timer,0);
  add_idt((uint64_t)timer, 32);
  add_idt((uint64_t)keyboard,33); 
 // Call LIDT
  load_idt(&idtr);
}

