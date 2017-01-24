#include <stdio.h>
#include "markdown.h"

static Paragraph *
mkd_h1(Paragraph *p)
{
    Paragraph *found;

    while ( p ) {
	if ( p->typ == HDR && p->hnumber == 1 )
	     return p;
       if ( p->down && (found = mkd_h1(p->down)) )
	   return found;
	p = p->next;
    }
    return 0;
}

char *
mkd_h1_title(Document *doc, int flags)
{
    Paragraph *title;

    if (doc && (title = mkd_h1(doc->code)) ) {
	  char *generated;
	  int size;

	  /* assert that a H1 header is one line long, so that's
	   * the only thing needed
	    */
	  size = mkd_line(T(title->text->text),
			  S(title->text->text), &generated, flags|MKD_TAGTEXT);
	  if ( size ) return generated;
    }
    return 0;
}
