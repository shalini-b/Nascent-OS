#include <sys/defs.h>

#ifndef _IDT_H
#define _IDT_H

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

extern void init_idt();
extern void add_idt(uint64_t func_base, int offset);
#endif
