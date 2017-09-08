#include <sys/defs.h>

#define __NR_write 1
#define __NR_exit 60
#define __NR_read 0
#define __NR_open 2
#define __NR_close 3
#define __NR_chdir 80
#define __NR_pipe 22
#define __NR_execve 59
#define __NR_fork 57
#define __NR_dup2 33
#define __NR_getcwd 79
#define __NR_wait4 61
#define __NR_access 21

#define syscall0(type, name) \
type name() \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "syscall \n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)) \
                             :"%rax"); \
    return (type) output; \
}

#define syscall1(type, name, type1, arg1) \
type name(type1 arg1) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t"\
                            "syscall \n\t" \
                            "movq  %%rax, %0 \n\t"\
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)) \
                             :"%rax", "%rdi"); \
    return (type) output; \
}

#define syscall2(type, name, type1, arg1, type2, arg2) \
type name(type1 arg1, type2 arg2) \
{ \
    long output; \
    __asm__ __volatile__(	"movq %1, %%rax \n\t" \
                            "movq %2, %%rdi \n\t" \
                            "movq %3, %%rsi \n\t" \
                            "syscall \n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long) (__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)) \
                             :"%rax", "%rdi", "%rsi"); \
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
                            "syscall \n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long) (__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)) \
                             :"%rax", "%rdi", "%rsi", "%rdx"); \
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
                            "syscall\n\t" \
                            "movq  %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)), \
                             "r" ((uint64_t)(arg4)) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10"); \
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
                            "syscall \n\t" \
                            "movq %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)), \
                             "r" ((uint64_t)(arg4)), \
                             "r" ((uint64_t)(arg5))) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8"); \
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
                            "syscall\n\t" \
                            "movq %%rax, %0 \n\t" \
                             :"=r" (output) \
                             :"r" ((long)(__NR_##name)), \
                             "r" ((uint64_t)(arg1)), \
                             "r" ((uint64_t)(arg2)), \
                             "r" ((uint64_t)(arg3)), \
                             "r" ((uint64_t)(arg4)), \
                             "r" ((uint64_t)(arg5)), \
                             "r" ((uint64_t)(arg6)) \
                             :"%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9"); \
    return (type) output; \
}


syscall3(ssize_t, write, int, fd, const void *, buf, size_t, count);
syscall3(ssize_t, read, int, fd, void *, buf, size_t, count);
syscall1(void, exit, int, value);
syscall2(int, open, const char *, pathname, int, flags);
syscall1(int, close, int, fd);
syscall1(int, chdir, const char*, fileName); 
syscall1(int, pipe, int *, filedes); 
syscall3(size_t, execve, const char *, fileName, char *const*, argv, char *const*, envp);
syscall0(pid_t, fork);
syscall2(char *, getcwd, char *, buf, unsigned long, size);
syscall2(int, dup2, int, oldfd, int, newfd);
//TODO: add rusage struct for wait4
syscall4(pid_t, wait4, pid_t, pid, int *, status, int, options, char*, rusage); 
syscall2(int, access, const char *, filename, int, mode);
