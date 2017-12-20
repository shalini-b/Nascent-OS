#include <stdio.h>
#include <syscalls.h>

int waitpid(int pid, int *status, int options){
    return wait_s(pid);
}

pid_t wait(int *status) {
    // HACK!! As we need to send a pid here
    // Do not consider this pid in wait syscall
    return wait_s(0);
}

