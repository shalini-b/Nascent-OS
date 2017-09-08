#include <stdio.h>
#include <unistd.h>
#include "syscalls.h"


#define MAX_PATH 1024

char * _get_first(const char *str, const char chr) {

  while (*str) {
    if (*str == chr) {
      return (char*)str;
    }

    str++;
  }
/*
  if (*str == ch) {
    return (char *) str;
  }
*/
  return NULL;

}

size_t _get_length(const char *str) {
  size_t len= 0;

  while (str[len]) {
    len ++;
  }

  return len;
}

char *  memcopy(char *dst, const char *src, size_t len ) {

  for (size_t i = 0; i < len; i++) {
    dst[i] = src[i];
  }

  return dst + len; 
   
}

int execvp(const char *file, char *const argv[], char *const envp[]) {
  
  if (_get_first(file, '/') != NULL) {
     
     //XXX: Call execve
     execve(file, argv, NULL);
     return -1;
  }

  //char * path = getenv("PATH");
  char * path = "/bin:/sbin:/home/";
  if (path == NULL) {
    return -1;
  }

  const char *temp;
  size_t currLength;
  size_t pathLength = _get_length(path);
  size_t fileLength = _get_length(file);

  char buffer[pathLength + fileLength + 1];

  
  for (const char *curr = path; ; curr = temp) {
    temp = _get_first(curr, ':');
    
    if (temp == NULL) {
      currLength = _get_length(curr);
    } else {
      currLength = temp - curr;
    }

    if (currLength == 0 || currLength > MAX_PATH) {
       continue;
    }
    char* ptr = memcopy(buffer, curr, currLength);
    *ptr = '/';
    ptr= memcopy(ptr+1, file, fileLength);
    *ptr = '\0';
    // call execve here
    // call execve here
   if (access(buffer, 0) == 0) {
     puts(buffer);
     puts(argv[0]); 
     execve(buffer, argv, NULL);

   }
  }
  
  return -1;
}
