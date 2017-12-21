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

int
run_command(char command_array[][150], int args_num)
{
    {
        // Handle args for background processes
        int background = 0;
        char *args[args_num + 1];
        for (int i = 0; i < args_num; i++)
        {
            if (i == args_num - 1 && command_array[i][0] == '&')
            {
                background = 1;
                args[args_num - 1] = NULL;
                break;
            }
            // Create a NULL terminated argument list for execvpe
            args[i] = command_array[i];
        }
        args[args_num] = NULL;

        int st;
        pid_t pid;
        pid = fork();
        if (pid == 0)
        {
            // Child process - Execute command
            return execvp(args[0], args, NULL);
        }
        else if (pid > 0)
        {
            // Parent process
            // Wait for child only when it is not background process
            if (!background)
            {
                waitpid(pid, &st, WUNTRACED);
            }
        }
        else
        {
            puts("ERROR: Failed to execute command.");
        }
        return 0;
    }
}
char *
getenv(char *key)
{
    int ary_itr = 0;
    if (str_compare(key, "PS1") == 0)
    {
        return PS1;
    }
    while (ary_itr < ENV_ARRAY_LENGTH)
    {
        if (str_compare(ENV_KEY[ary_itr], key) == 0)
            break;
        ary_itr++;
    }
    str_copy(ENV_VALUE[ary_itr], ENV_BUFFER);
    return ENV_BUFFER;
}

int
setenv(char *name, char *value, int overwrite)
{
    int ary_itr = 0;
    int found_flag = 0;
    while (ary_itr < ENV_ARRAY_LENGTH)
    {
        if (str_compare(ENV_KEY[ary_itr], name) == 0)
        {
            found_flag = 1;
            break;
        }
        ary_itr++;
    }
    if (found_flag)
    {
        str_copy(value, ENV_VALUE[ary_itr]);
    }
    else
    {
        ary_itr++;
        str_copy(name, ENV_KEY[ary_itr]);
        str_copy(value, ENV_VALUE[ary_itr]);
    }
    return 0;
}

int
command_handler(char command_array[][150], int args_num)
{
    int ret_code = 0;
    if (len(command_array[0]) == 0)
    {
        ret_code = 0;
    }
    if ((str_compare(command_array[0], "exit") == 0))
    {
        exit(0);
    }
    else if (str_compare(command_array[0], "cd") == 0)
    {
        if (chdir(command_array[1]) != 0)
        {
            puts("ERROR: Failed To Change Directory 2");
        }
    }
    else if (str_compare(command_array[0], "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
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
            str_copy(split_array[1], PS1);
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
        ret_code = run_command(command_array, args_num);
    }
    return ret_code;
}

void
clear_2darray(char array1[150][150])
{
    for (int i = 0; i < 150; i++)
    {
        for (int j = 0; j < 150; j++)
        {
            array1[i][j] = '\0';
        }
    }
}

int
main(int argc, char *argv[], char *envp[])
{
//    for(int i=0;i<5000;i++)
//    {
//        char* abc =(char*)malloc(5);
//        abc[1]='b';
////        free(abc);
//    }


    char string_buffer_array[INPUT_STRING_BUFFER_LENGTH];
    int ptr1 = 0;

    int t = 0;
    if (argc == 2)
    {
        int fd;
        fd = open(argv[1], 1);
        memset((void *) cmd_buff, '\0', CMD_BUFF_SIZE);
        while (read(fd, cmd_buff, CMD_BUFF_SIZE) != 0);
        close(fd);
    }
    while (1)
    {
        memset((void *) string_buffer_array, '\0', 100);
        clear_2darray(final_parsed_array);
        int c;
        int ret = 0;

        // Input command
        if (argc == 1)
        {
            printf("%s", PS1);
            gets(string_buffer_array);
            t = 2;
        }
        else
        {
            int i = 0;
            c = 0;
            while ((ptr1 < CMD_BUFF_SIZE) && (i < INPUT_STRING_BUFFER_LENGTH))
            {
                if (cmd_buff[ptr1] == '\n')
                {
                    string_buffer_array[i] = '\0';
                    ptr1++;
                    i++;
                    t++;
                    break;
                }
                else
                {
                    string_buffer_array[i] = cmd_buff[ptr1];
                    i++;
                    if (cmd_buff[ptr1] == '\0')
                    {
                        memset((void *) string_buffer_array, '\0', 100);
                        c = 1;
                        break;
                    }

                }
                ptr1++;

            }
        }
        if (c == 1)
        {
            break;
        }

        if (t >= 2)
        {
            if (str_compare(string_buffer_array, "") == 0) {
                continue;
            }
            int cnt = split_and_count(string_buffer_array, ' ', final_parsed_array);
            ret = command_handler(final_parsed_array, cnt + 1);
            if (ret == -1)
            {
                printf("Wrong command. Please try again.\n");
            }
        }

    }
    return 0;
}


