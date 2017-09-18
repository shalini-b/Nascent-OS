#ifndef _PIC_H
#define _PIC_H

#include <sys/defs.h>

void PIC_remap();
//void PIC_remap(int offset1, int offset2)
void PIC_sendEOI(uint8_t irq);

#endif
