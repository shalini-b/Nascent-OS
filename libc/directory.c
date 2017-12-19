#include<dirent.h>
#include <mem.h>
#include <syscalls.h>
#include<strings.h>
#include <stdio.h>

DIR dir_array[20];

DIR *
opendir(const char *name)
{
    int fd = open_dir((char *) name);
    dir_array[fd].fd = fd;
    return &dir_array[fd];

}
struct dirent *
readdir(DIR *dirp)
{
    char name[100];
    int fd = dirp->fd;
    memset((void *) name, 0, 100);
    int r = read_dir(fd, name);
    if (r != 0)
    {
        return NULL;
    }
    str_copy((char *) name, dirp->name.d_name);
    return &dirp->name;
}
int
closedir(DIR *dirp)
{
    int fd = dirp->fd;
    return close_dir(fd);
}

int
chdir(const char *path)
{
    chdir_s((char*)path);
    return 0;
}


char *getcwd(char *buf, size_t size)
{
    getcwd_s(buf,size);
    return buf;
}