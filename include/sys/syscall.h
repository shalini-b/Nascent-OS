#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#include <sys/task.h>

void transfer_to_ring3();
void syscall_handler(Registers * regs);

#define SYS_open        2
#define SYS_write       1

#endif