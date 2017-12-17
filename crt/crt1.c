#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

char ENV_KEY[150][150];
char ENV_VALUE[150][4096];
int ENV_ARRAY_LENGTH;

/*void cache_env( char *env_array[])
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
}*/

void _start(void) {
  // call main() and exit() here

  long resp;
  int a;
   __asm__ __volatile__(
                            "movq %%rsp, %0"
                             :"=r" (resp)
                             :
                             :"%rsp");
    a=*((int*)(resp+8));
    char** b=(char**)(resp+16);
    char** c=(char**)(a*8+8+resp+16);
//    cache_env(c);
    int exit_code = main(a,b,c);
//    int exit_code =  main();
    exit(exit_code);
}
