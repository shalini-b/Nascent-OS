#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscalls.h>
#include <strings.h>
#include <mem.h>

#define INPUT_STRING_BUFFER_LENGTH 100
int a = 0;
void dummy();

char final_parsed_array[150][150];
char PS1[150] = "sbush>";

char ENV_KEY[150][150];
char ENV_VALUE[150][4096];
int ENV_ARRAY_LENGTH;
char ENV_BUFFER[1024];

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
    // FIXME: use clone system
    pid = fork();
    dummy();
    if (pid == 0){
        // Child process - Execute command
        // FIXME: do error handling
        printf("I am child\n");
        while(1);
        execvp(args[0], args, NULL);
        //exit(0);
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

void dummy() {
    a++;
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
    while (1)
    {
        // char buff[150];
        // Clearup for next iterations
        // FIXME: isnt it implicit when u re-initialize?

        memset((void *) string_buffer_array, '\0', 100);
        clear_2darray(final_parsed_array);

        // Input command
	    if(argc==1)
        {
            printf("%s", PS1);
            gets(string_buffer_array);
            printf("value entered > %s\n", string_buffer_array);
        }
        /*else
        {
            if(fgets(string_buffer_array, INPUT_STRING_BUFFER_LENGTH, fp) == NULL)
            {
                break;
            }
        }*/

        int cnt = split_and_count(string_buffer_array, ' ', final_parsed_array);
        command_handler(final_parsed_array, cnt);
    }
    return 0;
}

/*

 #include <stdio.h>
#include <dirent.h>
#include <mem.h>
#include <unistd.h>
#include <syscalls.h>


int
main(int argc, char *argv[], char *envp[])
{

     while (1)
    {
        char buff[100];
        printf("sbush>");
        read(0,buff,100);
        int a = fork();
        if(a==0){
            execvp("/bin/ls", NULL, NULL);
            exit(0);
        }
        else{
            int st;
            waitpid(a, &st, 2);
        }
        printf("value entered > %s\n",buff);
        memset((void *) buff, '\0', 100);

    }


   //DIR
    volatile struct dirent *b ;
    int DIR_SIZE = 100;
    char dir_buff2[DIR_SIZE];
    DIR *a1=opendir("test1/test2/abc/pqr/");
    memset((void *) dir_buff2, '\0', DIR_SIZE);
    while (1)
    {

        b = readdir(a1);
        if(b==NULL)
        {
            break;
        }
        printf("%s\n", b->d_name);
        memset((void *) dir_buff2, '\0', DIR_SIZE);
    }
    closedir(a1);


    //FILES
    int BUFF_SIZE = 100;
    char buff[BUFF_SIZE];
    int fd;
    fd = open("test1/abc.txt",1);
    memset((void *) buff, '\0',BUFF_SIZE);
    while (read(fd, buff, BUFF_SIZE) != 0)
    {
        printf("%s", buff);
        memset((void *) buff, '\0', BUFF_SIZE);
    }
    close(fd);
    int pid = fork();
    printf("\n PID %d\n",pid);
    printf("Hello! I am parent");

    if (pid == 0) {
        printf("Hello! I am child");
        execvp("/bin/sbush", NULL, NULL);   
	    exit(0);
    }
    else{
        int st;
        waitpid(pid, &st, 2);
    }
    printf("Sbush exiting...");
    while(1);
}
 */
