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
#include <ctype.h>

#include "cstring.h"
#include "markdown.h"

typedef ANCHOR(Line) LineAnchor;

/* set up a line anchor for mkd_add()
 */
static Document*
new_Document()
{
    return calloc(sizeof(Document), 1);
}


/* add a line to the markdown input chain
 */
static void
queue(Document* a, Cstring *line)
{
    Line *p = calloc(sizeof *p, 1);
    unsigned char c;
    int xp = 0;
    int           size = S(*line);
    unsigned char *str = T(*line);

    CREATE(p->text);
    ATTACH(a->content, p);

    while ( size-- ) {
	if ( (c = *str++) == '\t' ) {
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


/* build a Document from any old input.
 */
typedef unsigned int (*getc_func)(void*);

Document *
populate(getc_func getc, void* ctx, int flags)
{
    Cstring line;
    Document *a = new_Document();
    int c;
#ifdef PANDOC_HEADER
    int pandoc = 0;
#endif

    if ( !a ) return 0;

    CREATE(line);

    while ( (c = (*getc)(ctx)) != EOF ) {
	if ( c == '\n' ) {
#ifdef PANDOC_HEADER
	    if ( pandoc != EOF && pandoc < 3 ) {
		if ( T(line)[0] == '%' )
		    pandoc++;
		else
		    pandoc = EOF;
	    }
#endif
	    queue(a, &line);
	    S(line) = 0;
	}
	else
	    EXPAND(line) = c;
    }

    if ( S(line) )
	queue(a, &line);

    DELETE(line);

#ifdef PANDOC_HEADER
    if ( (pandoc == 3) && !(flags & NO_HEADER) ) {
	/* the first three lines started with %, so we have a header.
	 * clip the first three lines out of content and hang them
	 * off header.
	 */
	a->headers = T(a->content);
	T(a->content) = a->headers->next->next->next;
	a->headers->next->next->next = 0;
    }
#endif

    return a;
}


/* convert a file into a linked list
 */
Document *
mkd_in(FILE *f, int flags)
{
    return populate((getc_func)fgetc, f, flags & INPUT_MASK);
}


/* return a single character out of a buffer
 */
struct string_ctx {
    char *data;		/* the unread data */
    int   size;		/* and how much is there? */
} ;


static char
strget(struct string_ctx *in)
{
    if ( !in->size ) return EOF;

    --(in->size);

    return *(in->data)++;
}


/* convert a block of text into a linked list
 */
Document *
mkd_string(char *buf, int len, int flags)
{
    struct string_ctx about;

    about.data = buf;
    about.size = len;

    return populate((getc_func)strget, &about, flags & INPUT_MASK);
}
