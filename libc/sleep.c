#include <unistd.h>
#include <syscalls.h>
unsigned int sleep(unsigned int seconds)
{
    sleep_s(seconds);
    return 0;
}