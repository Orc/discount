/*
 * xmlpage -- write a skeletal xhtml page
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
#include "amalloc.h"


int
mkd_xhtmlpage(Document *p, int flags, FILE *out)
{
    char *title;
    extern char *mkd_doc_title(Document *);
    
    if ( mkd_compile(p, flags) ) {
        int ret = 0;

#define MKD_ERRH(s) ret |= (s) < 0 ? -1 : 0

        MKD_ERRH( fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") );
        MKD_ERRH( fprintf(out, "<!DOCTYPE html "
             " PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
             " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n") );
        
        MKD_ERRH( fprintf(out, "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n") );
        
        MKD_ERRH( fprintf(out, "<head>\n") );
        if ( title = mkd_doc_title(p) )
            MKD_ERRH( fprintf(out, "<title>%s</title>\n", title) );
        MKD_ERRH( mkd_generatecss(p, out) );
        MKD_ERRH( fprintf(out, "</head>\n") );
        
        MKD_ERRH( fprintf(out, "<body>\n") );
        MKD_ERRH( mkd_generatehtml(p, out) );
        MKD_ERRH( fprintf(out, "</body>\n") );
        MKD_ERRH( fprintf(out, "</html>\n") );

#undef MKD_ERRH

        return ret;
    }
    return -1;
}
