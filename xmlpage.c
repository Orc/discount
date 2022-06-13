/*
 * xmlpage -- write a skeletal xhtml page
 *
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <markdown.h>

extern char *mkd_doc_title(Document *);
#if USE_H1TITLE
extern char* mkd_h1_title(Document *);
#endif

int
mkd_xhtmlpage(Document *p, mkd_flag_t* flags, FILE *out)
{
    if ( mkd_compile(p, flags) ) {
	DO_OR_DIE( fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<!DOCTYPE html "
				" PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
				" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n") );

	DO_OR_DIE( fprintf(out, "<head>\n") );
	DO_OR_DIE( fprintf(out, "<title>") );
#if USE_H1TITLE
	char *title = mkd_doc_title(p) || mkd_h1_title(p);
#else
	char *title = mkd_doc_title(p);
#endif
	if ( title )
	    DO_OR_DIE( fprintf(out, "%s", title) );
	DO_OR_DIE( fprintf(out, "</title>\n") );
	DO_OR_DIE( mkd_generatecss(p, out) );
	DO_OR_DIE( fprintf(out, "</head>\n"
				"<body>\n") );
	
	DO_OR_DIE( mkd_generatehtml(p, out) );
	DO_OR_DIE( fprintf(out, "</body>\n"
				"</html>\n") );

	return 0;
    }
    return EOF;
}
