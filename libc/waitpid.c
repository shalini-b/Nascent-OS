#include <stdio.h>
#include <syscalls.h>

int waitpid(int pid, int *status, int options){
return wait_s(pid);
}

int kill(pid_t pid, int sig)
{

    //FIXME:: implement syscall
    kill_s((int)pid);
    return 0;
}
