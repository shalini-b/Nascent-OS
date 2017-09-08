#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

char ENV_KEY[150][150];
char ENV_VALUE[150][4096];
int ENV_ARRAY_LENGTH;
char ENV_BUFFER[1024];
char PS1[150];

char* getenv(char* key)
{   
    int ary_itr=0;
    if(str_compare(key, "PS1")==0)
    {   
        return PS1;
    }
    while(ary_itr < ENV_ARRAY_LENGTH)
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
    while(ary_itr < ENV_ARRAY_LENGTH)
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

