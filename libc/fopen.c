#include <stdio.h>
#include <syscalls.h>
#include <unistd.h>

#define	O_RDONLY 0x0000		/* open for reading only */
#define	O_WRONLY 0x0001		/* open for writing only */
#define	O_RDWR 0x0002		/* open for reading and writing */
#define	O_ACCMODE 0x0003	/* mask for above modes */
#define	O_CREAT	0x0200		/* create if nonexistant */
#define	O_TRUNC	0x0400		/* truncate to zero length */
#define	O_APPEND 0x0008		/* set append mode */

FILE *fopen(const char *path, const char *mode)
{
  int fd;
  FILE out;
  FILE *pointer = &out;
//  pointer = File
  if (mode[0] == 'r' && mode[1] == '+') {
    // call open with some argument
  //  write(1, "reached here\n", 13);
    fd = open(path, O_RDWR);
  }
  else if (mode[0] == 'r') {
  //  write(1, "reached here1\n", 14);
    fd = open(path, O_RDONLY);
  }
  else if (mode[0] == 'w' && mode[1] == '+') {
  //  write(1, "reached here2\n", 14);
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC);
  }
  else if (mode[0] == 'w') {
  //  write(1, "reached here3\n", 14);
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
  }
  else if (mode[0] == 'a' && mode[1] == '+') {
  //  write(1, "reached here4\n", 14);
    fd = open(path, O_RDWR | O_CREAT | O_APPEND);
  }
  else if (mode[0] == 'a') {
    fd = open(path, O_WRONLY | O_CREAT | O_APPEND);
  } 
  else {
    
    return NULL;
  }

  if (fd == -1) {
  //  write(1, "reached here9\n", 14);
    return NULL;
  }

  pointer->fd = fd;
  pointer->base = "rand";
  pointer->cnt = 0;

  if (mode[0] == 'r') {
    pointer->flag = 1;
  } else {
    pointer->flag = 2;
  }
 
  return pointer;

}

