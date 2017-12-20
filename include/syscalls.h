
#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include <sys/defs.h>
#include<sys/types.h>


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


syscall1_def(int, open_dir, char*,name);
syscall2_def(int, read_dir, int,fd,char*,name);
syscall1_def(int, close_dir, int,fd);


syscall2_def(int, open_s, char*,name,int ,flags);
syscall3_def(int, read_s, int,fd,char*,name,int, count);
syscall1_def(int, close_s, int,fd);

syscall3_def(ssize_t, write, int, fd, const void *, buf, size_t, count);
syscall1_def(void, exit, int, value);
syscall3_def(size_t, execve, const char *, fileName,  char *const*, argv, char *const*, envp);
syscall2_def(int, dup2, int, oldfd, int, newfd);
syscall1_def(int,sleep_s, int, sec);
syscall1_def(pid_t, wait_s, pid_t, pid);
syscall2_def(int, access, const char *, filename, int, mode);
syscall0_def(void, ps);
syscall1_def(int,kill_s, int, pid);
syscall1_def(int,chdir_s, char*, path);
syscall2_def(char*,getcwd_s, char*, buf,size_t,size);

syscall0_def(void, sys_yield);
syscall1_def(void *, sys_sbrk, int, limit);

#endif
