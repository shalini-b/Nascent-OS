#include <sys/defs.h>
#include <unistd.h>
#define __NR_write 1
#define __NR_exit 60
#define __NR_read 0
#define __NR_open 2
#define __NR_close 3
// FIXME: can we remove this??
#define __NR_chdir 80
#define __NR_pipe 22
#define __NR_execve 59
#define __NR_fork 57
#define __NR_dup2 33
// FIXME: can we remove this??
#define __NR_getcwd 79
#define __NR_wait_s 61
#define __NR_access 21
#define __NR_getpid  39
#define __NR_getppid 110
#define __NR_open_dir 70
#define __NR_read_dir 71
#define __NR_close_dir 72
#define __NR_sleep_s 73
#define __NR_ps 75
#define __NR_kill_s 76
#define __NR_chdir_s 50
#define __NR_getcwd_s 51

// self defined - be careful with the numbers
#define __NR_sys_yield   99
#define __NR_sys_sbrk    100


#define syscall0(type, name) \
type name() \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "int $0x80 \n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)) \
                             :"%rax", "memory"); \
    return (type) output; \
}

#define syscall1(type, name, type1, arg1) \
type name(type1 arg1) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t"\
                            "int $0x80\n\t" \
                            "movq  %%rax, %0 \n\t"\
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)) \
                             :"%rax", "%rdi", "memory"); \
    return (type) output; \
}

#define syscall2(type, name, type1, arg1, type2, arg2) \
type name(type1 arg1, type2 arg2) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t" \
                            "movq %3, %%rsi \n\t" \
                            "int $0x80 \n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long) (__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)) \
                             :"%rax", "%rdi", "%rsi", "memory"); \
    return (type) output; \
}

#define syscall3(type, name, type1, arg1, type2, arg2, type3, arg3) \
type name(type1 arg1, type2 arg2, type3 arg3) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t" \
                            "movq %3, %%rsi \n\t" \
                            "movq %4, %%rdx \n\t" \
                            "int $0x80\n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long) (__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "memory"); \
    return (type) output; \
}

#define syscall4(type, name, type1, arg1, type2, arg2, type3, arg3, type4, arg4) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax\n\t" \
                            "movq %2, %%rdi\n\t" \
                            "movq %3, %%rsi\n\t" \
                            "movq %4, %%rdx\n\t" \
                            "movq %5, %%r10\n\t" \
                            "int $0x80\n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)), \
                             "r" ((uint64_t)(arg4)) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10", "memory"); \
    return (type) output; \
}

#define syscall5(type, name, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t" \
                            "movq %3, %%rsi \n\t" \
                            "movq %4, %%rdx \n\t" \
                            "movq %5, %%r10 \n\t" \
                            "movq %6, %%r8 \n\t" \
                            "int $0x80\n\t" \
                            "movq %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)), \
                             "r" ((uint64_t)(arg4)), \
                             "r" ((uint64_t)(arg5))) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "memory"); \
    return (type) output; \
}

#define syscall6(type, name, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5, type6, arg6) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t" \
                            "movq %3, %%rsi \n\t" \
                            "movq %4, %%rdx \n\t" \
                            "movq %5, %%r10 \n\t" \
                            "movq %6, %%r8 \n\t" \
                            "movq %7, %%r9 \n\t" \
                            "int $0x80\n\t" \
                            "movq %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)), \
                             "r" ((uint64_t)(arg4)), \
                             "r" ((uint64_t)(arg5)), \
                             "r" ((uint64_t)(arg6)) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9", "memory"); \
    return (type) output; \
}

//directory
syscall1(int, open_dir, char*,name);
syscall2(int, read_dir, int,fd,char*,name);
syscall1(int, close_dir, int,fd);
syscall3(ssize_t, read, int, fd, void *, buf, size_t, count);
syscall1(void, exit, int, value);
syscall2(int, open, const char *, pathname, int, flags);
syscall1(int, close, int, fd);
syscall1(int, pipe, int *, filedes); 
syscall3(size_t, execve, const char *, fileName, char *const*, argv, char *const*, envp);
syscall0(pid_t, fork);
syscall0(pid_t, getpid);
syscall0(pid_t, getppid);
syscall2(int, dup2, int, oldfd, int, newfd);
syscall3(ssize_t, write, int, fd, const void *, buf, size_t, count);
syscall1(pid_t, wait_s, pid_t, pid);
syscall0(void, ps);
syscall2(int, access, const char *, filename, int, mode);
syscall1(int,sleep_s, int, sec);
syscall1(int,kill_s, int, pid);
syscall1(int,chdir_s, char*, path);
syscall2(char*,getcwd_s, char*, buf,size_t,size);
syscall0(void, sys_yield);
syscall1(void *, sys_sbrk, int, limit);