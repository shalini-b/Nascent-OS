
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscalls.h>

#define INPUT_STRING_BUFFER_LENGTH 1024
char PS1[150] = "sbush>";
char ENV_KEY[150][150];
char ENV_VALUE[150][4096];
int ENV_ARRAY_LENGTH;
char ENV_BUFFER[1024];

//Length of string
int
len(char string[])
{
    int string_length = 0;
    while (string[string_length] != '\0')
    {
        string_length++;
    }
    return string_length;
}

int
str_compare(char s1[150], char s2[150])
{
    int str_itr = 0;
    if (len(s1) != len(s2))
        return 1;

    while (s1[str_itr] != '\0')
    {
        if (s1[str_itr] != s2[str_itr])
        {
            return 1;
        }
        str_itr++;
    }
    return 0;
}

void
str_copy(char *source_string, char *destination_string)
{
    int str_itr = 0;
    while (source_string[str_itr] != '\0')
    {
        destination_string[str_itr] = source_string[str_itr];
        str_itr++;
    }
    destination_string[str_itr] = '\0';
}

//Split Input String: returns count of given delimiter
//FIXME : can only split by single delimiter character
int
split_and_count(char *string_to_split, char delimiter, char final_array[][150])
{

    int string_itr = 0;
    int fin_arr_itr = 0;
    int block = 0;
    int found_flag = 0;
    int letter_flag = 0;
    if (!delimiter)
    {
        // FIXME: y do this?
        final_array = NULL;
    }
    // Parse till end of input
    while (string_to_split[string_itr] != '\0')
    {
        if (string_to_split[string_itr] != ' ')
        {
            // flag for marking chars 
            letter_flag = 1;
        }
        if (string_to_split[string_itr] == delimiter && letter_flag)
        {
            // flag for marking delimiter
            found_flag = 1;
        }
        if (found_flag && string_to_split[string_itr] != delimiter)
        {
            // deimiter found in prev iteration
            final_array[block][fin_arr_itr] = '\0';
            fin_arr_itr = 0;
            block++;
            found_flag = 0;
        }
        if (!found_flag && letter_flag && string_to_split[string_itr] != '\n')
        {
            // no delimiter found while parsing through chars. FIXME: y check for new line?
            final_array[block][fin_arr_itr] = string_to_split[string_itr];
            fin_arr_itr++;
        }
        string_itr++;

    }
    final_array[block][fin_arr_itr] = '\0';

    return block;
}

void
run_command(char command_array[][150], int inlet, int outlet, int args_num){
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
    // FIXME: use clone system
    pid = fork();
    if (pid == 0){
        // Child process

        // Reset stdin & stdout to default params to call execvpe
        if (inlet != 0){
            dup2(inlet, 0);
            close(inlet);
        }
        if (outlet != 1){
            dup2(outlet, 1);
            close(outlet);
        }

        // FIXME: Now call execvpe with env
        // Define env
       // char *path = getenv("PATH");
        //char  env[500];
        //sprintf(env, "PATH=%s", path);

        // Execute command
        // FIXME: do error handling
        execvp(args[0], args, NULL);
    }
    else if (pid > 0){
        // Parent process
        // Wait for child only when it is not background process
        if(!background) {
            do {
                waitpid(pid, &st, WUNTRACED);
            } while (!WIFSIGNALED(st) && !WIFEXITED(st));
        }
    }
    else{
        puts("ERROR: Failed to execute command.");
    }
}


char* getenv(char* key)
{
int ary_itr=0;
    while(ary_itr<ENV_ARRAY_LENGTH)
    {
        if(str_compare(ENV_KEY[ary_itr], key) == 0)
            break;
        ary_itr++;
    }
    str_copy(ENV_VALUE[ary_itr],ENV_BUFFER);
    return ENV_BUFFER;
}


void cache_env( char *env_array[])
{
    int ary_itr =0;
    while(env_array[ary_itr]!=NULL)
    {
        if(env_array[ary_itr][0]=='L' && env_array[ary_itr][1]=='S')
        {
            ary_itr++;
            continue;
        }
        char split_array[2][150];
        split_and_count(env_array[ary_itr], '=', split_array);
        str_copy(split_array[0],ENV_KEY[ary_itr]);
        str_copy(split_array[1],ENV_VALUE[ary_itr]);
        ary_itr++;
    }
    ENV_ARRAY_LENGTH = ary_itr;
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
command_handler(char command_array[][150], int fdin, int fdout, int args_num)
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
        if (setenv(split_array[0], split_array[1], 1))
        {
            puts("ERROR: Failed To Change ENV Variable");
        }
        if (str_compare(split_array[0], "PS1") == 0)
        {
            str_copy(split_array[1],PS1);
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
        run_command(command_array, fdin, fdout, args_num);
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

void clear_3darray(char array2[150][150][150])
{
    for(int i=0;i<150;i++)
    {
        for(int j=0;j<150;j++)
        {
            for (int k=0;k<150;k++)
            {
                array2[i][j][k] = '\0';
            }

        }
    }

}

int
main(int argc, char *argv[], char *envp[])
{
cache_env(envp);
    while (1)
    {
char temp_parsed_array[150][150];
        char final_parsed_array[150][150][150];
        char string_buffer_array[INPUT_STRING_BUFFER_LENGTH];
        int num_pipes = 0, cnt = 0;

        // Clearup for next iterations
        // FIXME: isnt it implicit when u re-initialize?
        clear_2darray(temp_parsed_array);
        clear_3darray(final_parsed_array);

        // Display shell prompt 
        puts(PS1);
        char* resp;
        // Input command
        resp = fgets(string_buffer_array, INPUT_STRING_BUFFER_LENGTH, stdin);
 if(resp== NULL)
        {
            puts("ERROR: Failed To Take Input");
        }

        // Split with pipe delimiter
        num_pipes = split_and_count(string_buffer_array, '|', temp_parsed_array);

        // Split with whitespace. FIXME: other delimiters?
        int cnt_args[num_pipes+1];
        for (int count = 0; count <= num_pipes; count++)
        {
            cnt = split_and_count(temp_parsed_array[count], ' ', final_parsed_array[count]);
            cnt_args[count] = cnt + 1;
        }

        // Execute each command with support for pipe
        int fdin = 0, fd[2];

        // Execute till last but one command
        for (int count = 0; count < num_pipes; count++)
        {
            if (pipe(fd) == -1)
            {
                puts("ERROR: Failure while creating pipe");
            }
            command_handler(final_parsed_array[count], fdin, fd[1], cnt_args[count]);
            close(fd[1]);
            fdin = fd[0];
        }

        // Calling command_handler for last command
        command_handler(final_parsed_array[num_pipes], fdin, fd[1], cnt_args[num_pipes]);
    }
    return 0;

}

