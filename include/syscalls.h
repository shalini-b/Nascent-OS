
#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include <sys/defs.h>

#define syscall0_def(type, name) \
type name();

#define syscall1_def(type, name, type1, arg1) \
type name(type1 arg1);

#define syscall2_def(type, name, type1, arg1, type2, arg2) \
type name(type1 arg1, type2 arg2);

#define syscall3_def(type, name, type1, arg1, type2, arg2, type3, arg3) \
type name(type1 arg1, type2 arg2, type3 arg3);

#define syscall4_def(type, name, type1, arg1, type2, arg2, type3, arg3, type4, arg4) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4);

#define syscall5_def(type, name, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5);


#define syscall6_def(type, name, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5, type6, arg6) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6);

syscall3_def(ssize_t, write, int, fd, const void *, buf, size_t, count);
syscall1_def(void, exit, int, value);
syscall3_def(size_t, execve, const char *, fileName,  char *const*, argv, char *const*, envp);
syscall2_def(int, dup2, int, oldfd, int, newfd);
syscall4_def(pid_t, wait4, pid_t, pid, int *, status, int, options, char*, rusage);
syscall2_def(int, access, const char *, filename, int, mode);

#endif
