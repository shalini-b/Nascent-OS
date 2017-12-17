#include <stdio.h>
#include <unistd.h>

char *
gets(char *s)
{

	read(0, s, 100);
	return s;

}
