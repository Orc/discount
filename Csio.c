#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cstring.h"
#include "markdown.h"
#include "amalloc.h"


/* putc() into a cstring
 */
void
Csputc(int c, Cstring *iot)
{
    EXPAND(*iot) = c;
}


/* printf() into a cstring
 */
int
Csprintf(Cstring *iot, char *fmt, ...)
{
    va_list ptr;
    int siz=100;

    do {
	RESERVE(*iot, siz);
	va_start(ptr, fmt);
	siz = vsnprintf(T(*iot)+S(*iot), ALL(*iot)-S(*iot), fmt, ptr);
	va_end(ptr);
    } while ( siz > (ALL(*iot)-S(*iot)) );

    S(*iot) += siz;
    return siz;
}


/* reparse() into a cstring
 */
void
Csreparse(Cstring *iot, char *buf, int size, int flags)
{
    MMIOT f;
    ___mkd_initmmiot(&f, 0);
    ___mkd_reparse(buf, size, 0, &f);
    ___mkd_emblock(&f);
    SUFFIX(*iot, T(f.out), S(f.out));
    ___mkd_freemmiot(&f, 0);
}
