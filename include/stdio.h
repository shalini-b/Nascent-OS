#ifndef _STDIO_H
#define _STDIO_H


typedef struct _iobuf {
  int fd;
  char *base;
  int cnt;
  int flag;
  char *ptr;
} FILE;
FILE _iob[3];

#define stdin	(&_iob[0])
#define stdout	(&_iob[1])
#define stderr	(&_iob[2])

static const int EOF = -1;

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);

char *gets(char *s);
FILE *fopen(const char *pathname, const char *mode);
char *fgets(char *res, int max, FILE *fp);

#endif
