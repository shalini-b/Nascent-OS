#include <types.h>
#include <unistd.h>
#include <dirent.h>
#include<stdio.h>
#include <syscalls.h>

int
main(int argc, char *argv[], char *envp[])
{
    ps();
    return 0;
}
