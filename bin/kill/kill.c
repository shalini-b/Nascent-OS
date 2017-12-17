#include <types.h>
#include <unistd.h>
#include <strings.h>
#include<signal.h>


int
main(int argc, char *argv[], char *envp[])
{
    kill(str_to_num(argv[1]),9);
    return 0;
}
