/*
 * toc -- spit out a table of contents based on header blocks
 *
 * Copyright (C) 2022 David L Parsons
 * portions Copyright (C) 2011 Stefano D'Angelo
 * Copyright (C) 2008 Jjgod Jiang, David L Parsons
 *
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

/* import from Csio.c */
extern void Csreparse(Cstring *, char *, int, mkd_flag_t*);

/* write an header index
 */
int
mkd_toc(Document *p, char **doc)
{
    Paragraph *tp, *srcp;
    int last_hnumber = 0;
    Cstring res;
    int size;
    int first = 1;
#if HAVE_NAMED_INITIALIZERS
    static mkd_flag_t islabel = { { [IS_LABEL] = 1 } };
#else
    mkd_flag_t islabel;

    mkd_init_flags(&islabel);
    set_mkd_flag(&islabel, IS_LABEL);
#endif


    if ( !(doc && p && p->ctx) ) return -1;

    *doc = 0;

    if ( ! is_flag_set(&p->ctx->flags, MKD_TOC) ) return 0;

    CREATE(res);
    RESERVE(res, 100);

    for ( tp = p->code; tp ; tp = tp->next ) {
	if ( tp->typ == SOURCE ) {
	    for ( srcp = tp->down; srcp; srcp = srcp->next ) {
		if ( (srcp->typ == HDR) && srcp->label ) {

		    while ( last_hnumber > srcp->hnumber ) {
			if ( (last_hnumber - srcp->hnumber) > 1 )
				Csprintf(&res, "\n");
			Csprintf(&res, "</li>\n%*s</ul>\n%*s",
				 last_hnumber-1, "", last_hnumber-1, "");
			--last_hnumber;
		    }

		    if ( last_hnumber == srcp->hnumber )
			Csprintf(&res, "</li>\n");
		    else if ( (srcp->hnumber > last_hnumber) && !first )
			Csprintf(&res, "\n");

		    while ( srcp->hnumber > last_hnumber ) {
			Csprintf(&res, "%*s<ul>\n", last_hnumber, "");
			if ( (srcp->hnumber - last_hnumber) > 1 )
			    Csprintf(&res, "%*s<li>\n", last_hnumber+1, "");
			++last_hnumber;
		    }
		    Csprintf(&res, "%*s<li><a href=\"#", srcp->hnumber, "");
		    mkd_string_to_anchor(srcp->label, strlen(srcp->label),
					 (mkd_sta_function_t)Csputc,
					 &res,1,p->ctx);
		    Csprintf(&res, "\">");
		    Csreparse(&res, T(srcp->text->text),
				    S(srcp->text->text), &islabel);
		    Csprintf(&res, "</a>");

		    first = 0;
		}
	    }
        }
    }

    while ( last_hnumber > 0 ) {
	--last_hnumber;
	Csprintf(&res, "</li>\n%*s</ul>\n%*s",
		 last_hnumber, "", last_hnumber, "");
    }

    if ( (size = S(res)) > 0 ) {
	/* null-terminate & strdup into a free()able memory chunk
	 */
	EXPAND(res) = 0;
	*doc = strdup(T(res));
    }
    DELETE(res);
    return size;
}


/*
 * rewrite *name so it doesn't collide with any of the header labels
 * in this document.
 */
static void
decollide(Paragraph *current, Cstring *name, int suffix)
{
    Paragraph *content;
    int needed, alloc;
    int seq = 0;
    char *sufp;

    /* first decollide all the children
     */
    for ( content = current; content; content = content->next ) {
	if ( content->down )
	    decollide(content->down, name, suffix);
    }

restart:
    for ( content = current; content; content = content->next ) {
	if ( content->typ == HDR && content->text && content->label ) {
	    if ( strcmp(T(*name), content->label) == 0 ) {
		/* collision; bump trailing sequence and try again
		  */
		alloc = ALLOCATED(*name)-suffix;
		sufp = T(*name) + suffix;
		needed = 1+snprintf(sufp, alloc, "_%d", seq);
		if ( needed > alloc ) {
		    RESERVE(*name, needed);
		    snprintf(sufp, needed, "_%d", seq);
		}
		++seq;
		goto restart;
	    }
	}
    }
    return;
}


/*
 * set up to run decollide on *name
 */
static char *
uniquename(ParagraphRoot *pr, Cstring *name)
{
    Cstring label;
    int suffix;
    char *final;

    suffix = S(*name);

    CREATE(label);
    RESERVE(label, suffix + 200);
    strcpy(T(label), T(*name));

    decollide(T(*pr), &label, suffix);

    final = strdup(T(label));
    DELETE(label);

    return final;
}


/*
 * assign unique names to all of the headers (with MKD_TOC; hand assigned
 * labels aren't examined)
 */
void
___mkd_uniquify(ParagraphRoot *pr, Paragraph *pp)
{
    Paragraph *content;


    if ( !(pr && pp) )
	return;

    /* unique all the headers at this level */
    for (content = pp; content; content = content->next) {
	if ( content->typ == SOURCE )
	    ___mkd_uniquify(pr, content->down);
	else if ( content->typ == HDR && T(content->text->text) )
	    content->label = uniquename(pr, &(content->text->text));
    }

#if 0
    /* unique all the children */
    for (content = pp; content; content = content->next)
	if ( content->down )
	    ___mkd_uniquify(pr, content->down);
#endif
}


/* write an header index
 */
int
mkd_generatetoc(Document *p, FILE *out)
{
    char *buf = 0;
    int sz = mkd_toc(p, &buf);
    int ret = EOF;

    if ( sz > 0 )
	ret = fwrite(buf, 1, sz, out);

    if ( buf ) free(buf);

    return (ret == sz) ? ret : EOF;
}
