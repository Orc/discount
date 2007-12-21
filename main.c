#include <stdio.h>
#include "mkdio.h"

main(int argc, char **argv)
{

    if ( (argc > 1) && !freopen(argv[1], "r", stdin) ) {
	perror(argv[1]);
	exit(1);
    }
    markdown(mkd_in(stdin), stdout, 0);
    exit(0);
}
