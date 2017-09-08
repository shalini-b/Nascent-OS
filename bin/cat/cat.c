//
// Created by mohan on 6/9/17.
//
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void cat(char* file_name)
{
    int file_descriptor;
    file_descriptor = open(file_name,0);
    char buffer[5120];

    while(read(file_descriptor, buffer, 5120) > 0)
    {
        puts(buffer);
    }
}

int main(int argc, char *argv[], char *envp[])
{
    cat(argv[1]);
return 0;
}
