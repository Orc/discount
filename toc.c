/*
 * toc -- spit out a table of contents based on header blocks
 *
 * Copyright (C) 2008 Jjgod Jiang, David L Parsons.
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

/* write an header index
 */
int
mkd_toc(Document *p, char **doc)
{
    Paragraph *pp;
    int last_hnumber = 0;
    Cstring res;
    
    CREATE(res);
    RESERVE(res, 100);

    *doc = 0;

    if ( !(p && p->ctx) ) return -1;
    if ( ! (p->ctx->flags & TOC) ) return 0;

    for ( pp = p->code; pp ; pp = pp->next ) {
        if ( pp->typ == HDR && pp->text ) {
	    
	    if ( last_hnumber == pp->hnumber )
		Csprintf(&res,  "%*s</li>\n", pp->hnumber, "");
	    else while ( last_hnumber > pp->hnumber ) {
		Csprintf(&res, "%*s</li>\n%*s</ul>\n",
				 last_hnumber, "",
				 last_hnumber-1,"");
		--last_hnumber;
	    }
	    
	    while ( pp->hnumber > last_hnumber ) {
		Csprintf(&res, "\n%*s<ul>\n", pp->hnumber, "");
		++last_hnumber;
	    }
	    Csprintf(&res, "%*s<li><a href=\"#", pp->hnumber, "");
	    mkd_string_to_anchor(T(pp->text->text), S(pp->text->text), Csputc, &res);
	    Csprintf(&res, "\">");
	    Csreparse(&res, T(pp->text->text), S(pp->text->text), 0);
	    Csprintf(&res, "</a>");
        }
    }

    while ( last_hnumber > 0 ) {
	Csprintf(&res, "%*s</li>\n%*s</ul>\n",
			last_hnumber, "", last_hnumber, "");
	--last_hnumber;
    }
			/* HACK ALERT! HACK ALERT! HACK ALERT! */
    *doc = T(res);	/* we know that a T(Cstring) is a character pointer */
			/* so we can simply pick it up and carry it away, */
    return S(res);	/* leaving the husk of the Ctring on the stack */
			/* END HACK ALERT */
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
	ret = fwrite(buf, sz, 1, out);

    if ( buf ) free(buf);

    return ret;
}
