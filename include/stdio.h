#ifndef _STDIO_H
#define _STDIO_H
extern char ENV_KEY[150][150];
extern char ENV_VALUE[150][4096];
extern int ENV_ARRAY_LENGTH;
extern char ENV_BUFFER[1024];
extern char PS1[150];

typedef struct _iobuf {
  int fd;
} FILE;
extern FILE _iob[3];
extern FILE ptr;

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

int setenv( char *name, char *value, int overwrite);
char* getenv(char* key);

#endif
