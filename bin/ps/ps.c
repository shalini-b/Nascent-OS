#include <types.h>
#include <unistd.h>
#include <dirent.h>
#include<stdio.h>
#include <syscalls.h>

void
ps1()
{
    ps();
}

int
main(int argc, char *argv[], char *envp[])
{
    ps1(argv[1]);
    return 0;
}
