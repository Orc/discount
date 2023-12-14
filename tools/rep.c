/*
 * repeat a string a bunch of times
 */
#include <stdio.h>
#include <stdlib.h>


int
main(argc, argv)
char **argv;
{
    int count;


    if ( argc < 3 ) {
	fprintf(stderr, "usage: rep <string> <count>\n");
	exit(1);
    }


    count = atoi(argv[2]);

    if ( count <= 0 ) {
	fprintf(stderr, "repeat count needs to be >0\n");
	exit(1);
    }

    while ( count-- > 0 )
	fputs(argv[1], stdout);


    exit(0);
}
