//
// Created by mohan on 6/9/17.
//
#include <stdio.h>
#include <types.h>
#include <unistd.h>
#include <mem.h>

void
cat(char *file_name)
{

    int BUFF_SIZE = 100;
    char buff[BUFF_SIZE];
    int fd;
    fd = open(file_name, 1);
    memset((void *) buff, '\0', BUFF_SIZE);
    while (read(fd, buff, BUFF_SIZE) != 0)
    {
        printf("%s", buff);
        memset((void *) buff, '\0', BUFF_SIZE);
    }
    close(fd);
}

int
main(int argc, char *argv[], char *envp[])
{
    cat(argv[1]);
    return 0;
}

