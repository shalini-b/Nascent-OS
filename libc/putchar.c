#include <stdio.h>
#include <syscalls.h>
#include <unistd.h>

int putchar(int c) {
  // write character to stdout
  
  char ch = c;
  int length = write(1, &ch, 1);
  if (length <= 0) {
    return EOF;
  }  

  return c;
}
