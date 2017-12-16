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
#define SYS_fork        57
#define SYS_getpid      39
#define SYS_getppid     110
#define SYS_open_dir    70
#define SYS_read_dir    71
#define SYS_close_dir   72
#define SYS_read        0
#define SYS_close       3
#define SYS_wait_s 61
#define SYS_exit 60
#define SYS_sleep_s 73
#define SYS_ps 75


#endif
