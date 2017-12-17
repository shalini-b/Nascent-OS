#include <types.h>
#include <unistd.h>
#include <dirent.h>
#include<stdio.h>
#include <syscalls.h>
#include <strings.h>

void
sleep1(int sec)
{
    sleep_s(sec);
}

int
main(int argc, char *argv[], char *envp[])
{
    sleep1(str_to_num(argv[1]));
    return 0;
}
