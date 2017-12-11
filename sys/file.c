#include <unistd.h>
#include<syscalls.h>


int open(const char *pathname, int flags)
{
    return open_s((char*)pathname,flags);
}


ssize_t read(int fd, void *buf, size_t count)
{
    return read_s( fd,(char*)buf,(int) count);
}

int close(int fd)
{
    return close_s(fd);
}
