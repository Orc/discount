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
populate(getc_func getc, void* ctx)
{
    Cstring line;
    Document *a = new_Document();
    int c;

    if ( !a ) return 0;

    CREATE(line);

    while ( (c = (*getc)(ctx)) != EOF ) {
	if ( c == '\n' ) {
	    queue(a, &line);
	    S(line) = 0;
	}
	else
	    EXPAND(line) = c;
    }

    if ( S(line) )
	queue(a, &line);

    DELETE(line);

    return a;
}


/* convert a file into a linked list
 */
Document *
mkd_in(FILE *f)
{
    return populate((getc_func)fgetc, f);
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
mkd_string(char *buf, int len)
{
    struct string_ctx about;

    about.data = buf;
    about.size = len;

    return populate((getc_func)strget, &about);
}
