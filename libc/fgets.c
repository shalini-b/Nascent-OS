#include <stdio.h>
#include <unistd.h>

char *
fgets(char *res, int max, FILE *fp)
{
	char letter, *pntr;
	for (pntr = res; max > 0; max--) {
		if (read (fp->fd, &letter, 1) == 0)
			break;
		*pntr++ = letter;
		if (letter == '\n')
			break;
	}
	*pntr = 0;
	return (pntr);
}

