#include <stdio.h>
#include <stdlib.h>

int
main()
{
    register int c;

    while ( (c=getchar()) != EOF) {
	if (c == ' ')
	    putchar('\n');
	else
	    putchar(c);
    }
    exit(0);
}
