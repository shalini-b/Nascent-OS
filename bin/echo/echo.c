#include <types.h>
#include <unistd.h>
#include<stdio.h>

void
echo(char* s )
{
    printf("%s ",s);
}

int
main(int argc, char *argv[], char *envp[])
{
    if (argc <= 1){
        printf("Invalid arguments\n");
    }

    for (int i = 1; i < argc;i++)
        echo(argv[i]);
    printf("\n");
    return 0;
}
