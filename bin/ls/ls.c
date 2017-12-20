#include <types.h>
#include <unistd.h>
#include <dirent.h>
#include<stdio.h>
#include <mem.h>
#include<syscalls.h>

void
ls(char* path)
{

    struct dirent *b;
    int DIR_SIZE = 100;
    char dir_buff2[DIR_SIZE];
    DIR *a1 = opendir(path);
    memset((void *) dir_buff2, '\0', DIR_SIZE);
    while (1)
    {
        b = readdir(a1);
        if (b == NULL)
        {
            break;
        }
        printf("%s\n", b->d_name);
        memset((void *) dir_buff2, '\0', DIR_SIZE);
    }
    closedir(a1);
}

int
main(int argc, char *argv[], char *envp[])
{
    // FIXME: handle no arguments to ls
    if(argc==1)
    {
       char cwd[100];

        if(getcwd(cwd, sizeof(cwd))== NULL)
        {
            puts("ERROR: Could Not Find CWD");
        }
        ls(cwd);
    }
    else
    {
        ls(argv[1]);
    }
    return 0;
}
