#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
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
	siz = vsnprintf(T(*iot)+S(*iot), ALLOCATED(*iot)-S(*iot), fmt, ptr);
	va_end(ptr);
    } while ( siz > (ALLOCATED(*iot)-S(*iot)) );

    S(*iot) += siz;
    return siz;
}


/* write() into a cstring
 */
int
Cswrite(Cstring *iot, char *bfr, int size)
{
    RESERVE(*iot, size);
    memcpy(T(*iot)+S(*iot), bfr, size);
    S(*iot) += size;
    return size;
}


/* strip () a cstring
 */
Cstring
Csstrip(Cstring cs)
{
    /* Trim leading whitespace */
    for ( ; S(cs); --S(cs) ) {
	if ( isspace(*T(cs)) ) {
	    ++T(cs);
	    --S(cs);
	}
	else
	    break;
    }

	/* Trim trailing whitespace */
    while ( S(cs) ) {
	if ( isspace(*(T(cs) + S(cs) - 1)) )
	    --S(cs);
	else
	    break;
    }

    return cs;
}


/* Return cstring keyword
 */
Cstring
Cskeyword(Cstring cs)
{
    Cstring token = cs;
    char *ptr = T(token);

    S(token) = 0;
    for ( register int i = 0; i < S(cs); ++i, ++ptr ) {
	if ( isalpha (*ptr) )
	    ++S(token);
	else
	    break;
    }

    return token;
}


/* Skip prefix cstring
 */
Cstring
Csskipprefix(Cstring base, Cstring prefix)
{
    char *prefixEnd = T_END(prefix);
    char *baseEnd = T_END(base);
    Cstring after;

    if ( T(prefix) >= T(base) && prefixEnd <= baseEnd ) {
	T(after) = ++prefixEnd;
	S(after) = S(base) - (prefixEnd - T(base));
    }
    else {
	T(after) = (char *) NULL;
	S(after) = 0;
    }

    return after;
}


/* reparse() into a cstring
 */
void
Csreparse(Cstring *iot, char *buf, int size, mkd_flag_t* flags)
{
    MMIOT f;
    ___mkd_initmmiot(&f, 0);
    ___mkd_reparse(buf, size, flags, &f, 0);
    ___mkd_emblock(&f);
    SUFFIX(*iot, T(f.out), S(f.out));
    ___mkd_freemmiot(&f, 0);
}
