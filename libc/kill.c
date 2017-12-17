#include<signal.h>
#include <syscalls.h>
int kill(pid_t pid, int sig)
{

    //FIXME:: implement syscall
    kill_s((int)pid);
    return 0;
}