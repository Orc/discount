/*
 * check a filename to see if it's a (fifo|character special|socket) object
 * (if a stat() function doesn't exist, we can't stat so we'll just return
 *  true no matter what.)
 */

#include "config.h"

#if HAVE_STAT && HAS_ISCHR && HAS_ISFIFO && HAS_ISSOCK
#include <sys/stat.h>

int
notspecial(char *file)
{
    struct stat info;

    if ( stat(file, &info) != 0 )
	return 1;
    
    return S_ISREG(info.st_mode);
}
#else
int
notspecial(char *file)
{
    return 1;
}
#endif


#if DEBUG

#include <stdio.h>

int
main(int argc, char **argv)
{
    int i;

    for ( i=1; i < argc; i++ )
	printf("%s is %sspecial\n", argv[i], notspecial(argv[i]) ? "not " : "");
}
#endif
