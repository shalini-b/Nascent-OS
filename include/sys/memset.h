#ifndef _MEM_H
#define _MEM_H
void *
memset(void *string_to_memset, int char_to_memset_with, int length_to_memset);
void
memcopy(void *source, void *destination, uint64_t size);
#endif