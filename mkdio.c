#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include "cstring.h"
#include "markdown.h"

typedef ANCHOR(Line) LineAnchor;

/* set up a line anchor for mkd_add()
 */
LineAnchor*
mkd_open()
{
    LineAnchor* p;

    return calloc(sizeof *p, 1);
}


/* add a line to the markdown input chain
 */
void
mkd_write(LineAnchor* a, char *s, int len)
{
    Line *p = calloc(sizeof *p, 1);
    char c;
    int i, xp;

    CREATE(p->text);
    ATTACH(*a, p);

    for (i=xp=0; i < len; i++) {
	if ( (c = s[i]) == '\t' ) {
	    /* expand tabs into 1..4 spaces.  This is not
	     * the traditional tab spacing, but the language
	     * definition /really really/ wants tabs to be
	     * 4 spaces wide (indents are in terms of tabs
	     * *or* 4 spaces.
	     */
	    do {
		EXPAND(p->text) = ' ';
	    } while ( ++xp & 03 );
	}
	else if ( c >= ' ' ) {
	    EXPAND(p->text) = c;
	    ++xp;
	}
    }
    EXPAND(p->text) = 0;
    S(p->text)--;
    p->dle = mkd_firstnonblank(p);
}


/* finish attaching input, return the
 * input chain.
 */
Line*
mkd_close(LineAnchor *p)
{
    Line *ret = T(*p);

    free(p);

    return ret;
}


/* read in the markdown source document, assemble into a linked
 * list.
 */
Line *
mkd_in(FILE *input)
{
    int i, c, xp;
    Line *p;
    static Cstring line = { 0, 0 };
    LineAnchor *a = mkd_open();

    if ( !a ) return 0;

    for (; (c = getc(input)) != EOF; ) {
	if (c == '\n') {
	    xp = 0;
	    mkd_write(a, T(line), S(line));
	    S(line) = 0;
	}
	else {
	    EXPAND(line) = c;
	    xp++;
	}
    }
    if ( xp )
	mkd_write(a, T(line), S(line));

    return mkd_close(a);
}
