/*
 * repeat a string a bunch of times
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *
deformat(char *s)
{
    char *ptr, *new = calloc(strlen(s)+1, 1);

    if ( (ptr = new) == NULL ) {
	fprintf(stderr, "out of memory!\n");
	exit(1);
    }
    for ( ; *s; s++ ) {
	if ( *s == '\\' && s[1] ) {
	    switch ( *(++s) ) {
		case 'n':  *ptr++ = '\n'; continue;
		case 'r':  *ptr++ = '\r'; continue;
		case 't':  *ptr++ = '\t'; continue;
		case 'b':  *ptr++ = '\b'; continue;
		case '\\': *ptr++ = '\\'; continue;
		default :  *ptr++ = '\\'; break;
	    }
	}
	*ptr++ = *s;
    }
    *ptr++ = 0;
    return new;
}


int
main(argc, argv)
char **argv;
{
    int count;
    char *string;
    char *prefix = NULL;
    char *suffix = NULL;


    if ( argc < 3 ) {
	fprintf(stderr, "usage: rep [prefix] <string> <count> [suffix]\n");
	exit(1);
    }


    switch (argc) {
    case 0: case 1: case 2:
	fprintf(stderr, "repeat count needs to be >0\n");
	exit(1);
    case 3:
	string = deformat(argv[1]);
	count = atoi(argv[2]);
	break;
    default:
	suffix = deformat(argv[4]);
    case 4:
	prefix = deformat(argv[1]);
	string = deformat(argv[2]);
	count = atoi(argv[3]);
	break;
    }

    if ( prefix ) {
	fputs(prefix, stdout);
	free(prefix);
    }

    while ( count-- > 0 )
	fputs(string, stdout);

    if ( suffix ) {
	fputs(suffix, stdout);
	free(suffix);
    }

    free(string);
    exit(0);
}
