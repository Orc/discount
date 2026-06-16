#include <stdio.h>
#include <stdlib.h>

main(argc, argv)
int argc;
char **argv;
{
    int reps;
    int i;
    char *pattern1, *text = NULL, *pattern2 = NULL;

    if ( argc < 3 ) {
	fprintf(stderr, "usage: 300 reps pattern1 [text [pattern2]]\n");
	exit(1);
    }


    reps = atoi(argv[1]);
    pattern1 = argv[2];
    if ( argc > 3 )
	text = argv[3];
    if ( argc > 4 )
	pattern2 = argv[4];

    for ( i=0; i < reps; i++ )
	printf("%s", pattern1);

    if ( text )
	printf(" %s ", text);

    if ( pattern2 )
	for ( i=0; i < reps; i++ )
	    printf("%s", pattern2);

    putchar('\n');
}
