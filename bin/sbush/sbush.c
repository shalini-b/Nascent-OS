#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscalls.h>
#include <strings.h>
#include <mem.h>

#define INPUT_STRING_BUFFER_LENGTH 100

char final_parsed_array[150][150];
char PS1[150] = "sbush>";

char ENV_KEY[150][150];
char ENV_VALUE[150][4096];
int ENV_ARRAY_LENGTH;
char ENV_BUFFER[1024];
#define CMD_BUFF_SIZE   4096
char cmd_buff[CMD_BUFF_SIZE];


void
run_command(char command_array[][150], int args_num){
    // Handle args for background processes
    int background = 0;
    char *args[args_num+1];
    for (int i=0; i<args_num; i++){
        if (i == args_num - 1 && command_array[i][0] == '&') {
            background = 1;
            args[args_num - 1] = NULL;
            break;
        }
        // Create a NULL terminated argument list for execvpe
        args[i] = command_array[i];
    }
    args[args_num] = NULL;

    int  st;
    pid_t pid;
    pid = fork();
    if (pid == 0){
        // Child process - Execute command
        // FIXME: do error handling
        execvp(args[0], args, NULL);
    }
    else if (pid > 0){
        // Parent process
        // Wait for child only when it is not background process
        if(!background) {
            waitpid(pid, &st, WUNTRACED);
        }
    }
    else{
        puts("ERROR: Failed to execute command.");
    }
}

char* getenv(char* key)
{
    int ary_itr=0;
    if(str_compare(key,"PS1")==0)
    {
        return PS1;
    }
    while(ary_itr<ENV_ARRAY_LENGTH)
    {
        if(str_compare(ENV_KEY[ary_itr], key) == 0)
            break;
        ary_itr++;
    }
    str_copy(ENV_VALUE[ary_itr],ENV_BUFFER);
    return ENV_BUFFER;
}

int setenv( char *name, char *value, int overwrite)
{
    int ary_itr=0;
    int found_flag=0;
    while(ary_itr<ENV_ARRAY_LENGTH)
    {
        if(str_compare(ENV_KEY[ary_itr], name) == 0)
        {
            found_flag =1;
            break;
        }
        ary_itr++;
    }
    if(found_flag)
    {
        str_copy(value,ENV_VALUE[ary_itr]);
    }
    else
    {
        ary_itr++;
        str_copy(name,ENV_KEY[ary_itr]);
        str_copy(value,ENV_VALUE[ary_itr]);
    }
    return 0;
}


int
command_handler(char command_array[][150], int args_num)
{
    if (len(command_array[0]) == 0)
    {
        return 0;
    }
    if ((str_compare(command_array[0], "exit") == 0))
    {
        exit(0);
    }
    else if (str_compare(command_array[0], "cd") == 0)
    {
        if (chdir(command_array[1])!=0)
        {
            puts("ERROR: Failed To Change Directory 2");
        }
    }
    else if (str_compare(command_array[0], "pwd") == 0)
    {
        char cwd[1024];
        if(getcwd(cwd, sizeof(cwd))!= NULL)
        {
            puts(cwd);
        }
        else
        {
            puts("ERROR: Could Not Find CWD");
        }
    }
    else if (str_compare(command_array[0], "export") == 0)
    {
        // Set environment variable
        char split_array[2][150];
        split_and_count(command_array[1], '=', split_array);
        if (str_compare(split_array[0], "PS1") == 0)
        {

            setenv(split_array[0], split_array[1], 1);
            str_copy(split_array[1],PS1);
        }
        else
        {
            setenv(split_array[0], split_array[1], 1);
        }
    }
    else if (str_compare(command_array[0], "getenv") == 0)
    {
        // Output PATH value
        puts(getenv(command_array[1]));
    }
    else
    {
        // fork & execvpe goes here with pipe support
        run_command(command_array, args_num);
    }
    return 0;
}

void clear_2darray(char array1[150][150])
{
    for(int i=0;i<150;i++)
    {
        for(int j=0;j<150;j++)
        {
            array1[i][j] = '\0';
        }
    }
}

int
main(int argc, char *argv[], char *envp[])
{
    char string_buffer_array[INPUT_STRING_BUFFER_LENGTH];
    /*FILE *fp = NULL;
    if(argc==2)
    {
        fp = fopen(argv[1],"r+");
    }*/
    /*if(argc==2) {
        int fd;
        fd = open(argv[1], 1);
        memset((void *) cmd_buff, '\0', CMD_BUFF_SIZE);
        while (read(fd, cmd_buff, CMD_BUFF_SIZE) != 0) {
            printf("%s", cmd_buff);
            memset((void *) cmd_buff, '\0', CMD_BUFF_SIZE);
        }
        close(fd);
    }*/
    while (1)
    {
        memset((void *) string_buffer_array, '\0', 100);
        clear_2darray(final_parsed_array);

        // Input command
	    if(argc==1)
        {
            printf("%s", PS1);
            gets(string_buffer_array);
        }
        /*else
        {

            for(int i=0;i<CMD_BUFF_SIZE;i++)
            {
                if(cmd_buff[i]=='\n')
                {

                }
                else if(cmd_buff[i]==' ')
                {

                }
                else
                {

                }

            }
        }*/
        int cnt = split_and_count(string_buffer_array, ' ', final_parsed_array);
        command_handler(final_parsed_array, cnt+1);


    }
    return 0;
}


