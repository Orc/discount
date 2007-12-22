/*
 * mkdio -- markdown front end input functions
 *
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include "cstring.h"
#include "markdown.h"

typedef ANCHOR(Line) LineAnchor;

/* set up a line anchor for mkd_add()
 */
static LineAnchor*
mkd_open()
{
    LineAnchor* p;

    return calloc(sizeof *p, 1);
}


/* add a line to the markdown input chain
 */
static void
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
static Line*
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
    int c;
    Cstring line;
    LineAnchor *a = mkd_open();

    if ( !a ) return 0;

    CREATE(line);
    for (; (c = getc(input)) != EOF; ) {
	if (c == '\n') {
	    mkd_write(a, T(line), S(line));
	    S(line) = 0;
	}
	else {
	    EXPAND(line) = c;
	}
    }
    if ( S(line) )
	mkd_write(a, T(line), S(line));

    DELETE(line);
    return mkd_close(a);
}


/* convert a block of text into a linked list
 */
Line *
mkd_string(char *buf, int len)
{
    Cstring line;
    LineAnchor *a = mkd_open();

    if ( !a ) return 0;

    CREATE(line);
    for ( ; len-- > 0; ++buf ) {
	if ( *buf == '\n' ) {
	    mkd_write(a, T(line), S(line));
	    S(line) = 0;
	}
	else
	    EXPAND(line) = *buf;
    }
    if ( S(line) )
	mkd_write(a, T(line), S(line));

    DELETE(line);
    return mkd_close(a);
}
