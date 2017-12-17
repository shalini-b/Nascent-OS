#include <unistd.h>
#include <syscalls.h>
#include <stdio.h>
#include <types.h>
#include <unistd.h>
#include <mem.h>
#include <strings.h>

#define MAX_PATH 1024

char *
_get_first(const char *str, const char chr)
{

    while (*str)
    {
        if (*str == chr)
        {
            return (char *) str;
        }

        str++;
    }
    return NULL;

}

size_t
_get_length(const char *str)
{
    size_t len = 0;

    while (str[len])
    {
        len++;
    }

    return len;
}

char *
memcopy1(char *dst, const char *src, size_t len)
{

    for (size_t i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }

    return dst + len;

}

int
execvp(const char *file, char *const argv[], char *const envp[])
{

    //read file
    int BUFF_SIZE = 30;
    char buff[BUFF_SIZE];
    int fd;
    fd = open((char *) file, 1);
    memset((void *) buff, '\0', BUFF_SIZE);
    read(fd, buff, BUFF_SIZE);
    close(fd);

    if (buff[0] == '#' && buff[1] == '!')
    {
        file = &buff[2];
        for (int i = 0; i < BUFF_SIZE; i++)
        {
            if (buff[i] == '\n')
            {
                buff[i] = '\0';
            }
        }
    }
    char a[50][50];
    str_copy("bin/sbush",a[0]);
    if (_get_first(file, '/') != NULL)
    {
        execve(file, (char**)a, envp);
        return -1;
    }

    char *path = getenv("PATH");
//    char *path = "/bin:/sbin:/home/";
    if (path == NULL)
    {
        return -1;

    }

    const char *temp;
    size_t currLength;
    size_t pathLength = _get_length(path);
    size_t fileLength = _get_length(file);

    char buffer[pathLength + fileLength + 1];

    int found = 0;
    const char *curr;
    for (curr = path; found != 1; curr = temp)
    {
        temp = _get_first(curr, ':');

        if (temp == NULL)
        {
            currLength = _get_length(curr);
        }
        else
        {
            currLength = temp - curr;
        }

        if (currLength == 0 || currLength > MAX_PATH)
        {
            continue;
        }
        char *ptr = memcopy1(buffer, curr, currLength);
        *ptr = '/';
        ptr = memcopy1(ptr + 1, file, fileLength);
        *ptr = '\0';
        // call execve here
        if (access(buffer, 0) == 0)
        {
            found = 1;
            execve(buffer, argv, NULL);
        }
    }

    char *ptr = memcopy1(buffer, curr, currLength);
    *ptr = '/';
    ptr = memcopy1(ptr + 1, file, fileLength);
    *ptr = '\0';
    // call execve here
    if (access(buffer, 0) == 0)
    {
        found = 1;
        execve(buffer, argv, envp);
    }
    return -1;
}
