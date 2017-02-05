/*
 * check a filename to see if it's a (fifo|character special|socket) object
 * (if a stat() function doesn't exist, we can't stat so we'll just return
 *  true no matter what.)
 */

#include "config.h"

#if HAVE_STAT
#include <sys/stat.h>

int
notspecial(char *file)
{
    struct stat info;

    if ( stat(file, &info) != 0 )
	return 1;
    
    return !(info.st_mode & (S_IFIFO|S_IFCHR|S_IFSOCK));
}
#else
int
notspecial(char *file)
{
    return 1;
}
#endif
