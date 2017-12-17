#include <types.h>
#include <unistd.h>
#include<stdio.h>

void
echo(char* s )
{
    printf("\n%s",s);

}

int
main(int argc, char *argv[], char *envp[])
{
    echo(argv[1]);
    return 0;
}
