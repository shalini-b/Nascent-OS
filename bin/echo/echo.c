#include <types.h>
#include <unistd.h>
#include<stdio.h>

void
echo(char* s )
{
    printf("%s\n",s);
}

int
main(int argc, char *argv[], char *envp[])
{
    echo(argv[1]);
    return 0;
}
