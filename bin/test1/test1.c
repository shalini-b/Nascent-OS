#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <string.h>

int atoi(char* s, int* num)
{
   int sign = 1;

   *num = 0;
   if (*s == '-')
   {
      sign = -1;
      s++;
   }

   while (*s)
   {
      if (*s < '0' || *s > '9')
         return 0;
      *num = (*num)*10 + (*s - '0');
      s++;
   }
  
   *num = (*num)*sign;
   return 1;
}

void lsleep(int secs)
{
    while(secs--)
      for (int i = 0;i < 3000;i++)
         for (int j = 0;j < 100000;j++);
}

int main(int argc, char* argv[], char* envp[])
{
   if (argc == 1)
   {
      printf("Should pass atleast one argument\n");
      exit(-1);
   }

   int count;
   if (!atoi(argv[1], &count))
   {
      printf("Invalid number\n");
      exit(-1);
   }


   int pid = getpid();
   int ppid = getppid();
   printf("pid %d, ppid %d START\n", pid, ppid);
   for (int i = 0;i < count ;i++)
   {
      lsleep(1);
      int pid = getpid();
      int ppid = getppid();
      printf("pid %d, ppid %d cnt - %dAWAKE\n", pid, ppid, i);
      //yield();
   } 
   return 0;
}
