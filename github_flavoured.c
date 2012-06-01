
/*
 * github_flavoured -- implement the obnoxious "returns are hard newlines"
 *                     feature in github flavoured markdown.
 *
 * Copyright (C) 2012 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "cstring.h"
#include "markdown.h"
#include "amalloc.h"

/* build a Document from any old input.
 */
typedef int (*getc_func)(void*);

Document *
gfm_populate(getc_func getc, void* ctx, int flags)
{
    Cstring line;
    Document *a = __mkd_new_Document();
    int c;
    int pandoc = 0;

    if ( !a ) return 0;

    a->tabstop = (flags & MKD_TABSTOP) ? 4 : TABSTOP;

    CREATE(line);

    while ( (c = (*getc)(ctx)) != EOF ) {
	if ( c == '\n' ) {
	    if ( pandoc != EOF && pandoc < 3 ) {
		if ( S(line) && (T(line)[0] == '%') )
		    pandoc++;
		else
		    pandoc = EOF;
	    }
            
            if (pandoc == EOF) {
		EXPAND(line) = ' ';
		EXPAND(line) = ' ';
	    }
	    __mkd_enqueue(a, &line);
	    S(line) = 0;
	}
	else if ( isprint(c) || isspace(c) || (c & 0x80) )
	    EXPAND(line) = c;
    }

    if ( S(line) )
	__mkd_enqueue(a, &line);

    DELETE(line);

    if ( (pandoc == 3) && !(flags & (MKD_NOHEADER|MKD_STRICT)) ) {
	/* the first three lines started with %, so we have a header.
	 * clip the first three lines out of content and hang them
	 * off header.
	 */
	Line *headers = T(a->content);

	a->title = headers;             __mkd_header_dle(a->title);
	a->author= headers->next;       __mkd_header_dle(a->author);
	a->date  = headers->next->next; __mkd_header_dle(a->date);

	T(a->content) = headers->next->next->next;
    }

    return a;
}


/* convert a block of text into a linked list
 */
Document *
gfm_string(const char *buf, int len, DWORD flags)
{
    struct string_stream about;

    about.data = buf;
    about.size = len;

    return gfm_populate((getc_func)__mkd_io_strget, &about, flags & INPUT_MASK);
}


/* convert a file into a linked list
 */
Document *
gfm_in(FILE *f, DWORD flags)
{
    return gfm_populate((getc_func)fgetc, f, flags & INPUT_MASK);
}
