#ifndef _MEM1_H
#define _MEM1_H
#include<types.h>
void *
memset(void *string_to_memset, int char_to_memset_with, int length_to_memset);
void
memcopy(void *source, void *destination, uint64_t size);
#endif