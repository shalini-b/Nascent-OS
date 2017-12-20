#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <string.h>

#define COUNT 5

int main(int argc, char* argv[], char* envp[])
{
   int i;

   for (i = 0;i < COUNT;i++)
   {
      int pid = fork();
      if (pid == 0)
      {
         char* argv[10] = {"bin/test1", "3", 0};
         execvp("bin/test1", argv, envp);
      }
   }

   printf("In Parent\n");
   for (i = 0;i < COUNT;i++)
   {
      int pid = wait(NULL);
      printf("pid exited - %d\n",pid);
   }

   return 0;
}
