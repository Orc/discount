/*
 * toc -- spit out a table of contents based on header blocks
 *
 * Copyright (C) 2008 Jjgod Jiang, David L Parsons
 * portions Copyright (C) 2011 Stefano D'Angelo
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
		if ( (srcp->typ == HDR) && srcp->text ) {
	    
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


char *
___mkd_uniquetag(ParagraphRoot *pr, char *name, int length)
{
    Paragraph *content;
    char *label;
    int suffix;
    int seq = 0;

    if ( !(pr && name) ) return 0;

    
    suffix = length;
    
    label = calloc(1, suffix+20);

    strcpy(label, name);

restart:
    for ( content = T(*pr); content; content = content->next ) {
	
	if ( content->text && content->typ == HDR ) {

	    if ( content->label && strcmp(label, content->label) == 0 ) {
		/* collision; bump trailing sequence and try again
		  */
		sprintf(label+suffix, "_%d", ++seq);
		goto restart;
	    }
	}
    }
    return label;
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
