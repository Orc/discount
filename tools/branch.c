#include <stdio.h>
#include <string.h>

#include "config.h"

int
main(argc, argv)
int argc;
char **argv;
{
    char *root;

    if ( argc <= 1 )
	return 0;

    if ( root = strrchr(argv[1], '/') )
	++root;
    else
	root = argv[1];

    if ( strcmp(root, "master" ) != 0 )
	printf("\"(%s)\"", root);

    return 0;
}
