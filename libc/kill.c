#include <signal.h>
#include <syscalls.h>

int kill(pid_t pid, int sig)
{
    kill_s((int)pid);
    return 0;
}