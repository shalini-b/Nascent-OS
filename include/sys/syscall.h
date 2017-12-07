#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#include <sys/process.h>

void transfer_to_ring3();
//void syscall_handler(Registers * regs);
uint64_t
syscall_handler(Registers1 *regs);
uint64_t
write_to_console(uint64_t fd, char *buffer, uint64_t count);

#define SYS_open        2
#define SYS_write       1

#endif
