/*
 * mkd2html:  parse a markdown input file and generate a web page.
 *
 * usage:  mkd2html [options] filename
 *  or     mkd2html [options] < markdown > html
 *
 *  options
 *         -css css-file
 *         -header line-to-add-to-<HEADER>
 *         -footer line-to-add-before-</BODY>
 *
 * example:
 *
 *   mkd2html -css /~orc/pages.css syntax
 *     ( read syntax OR syntax.text, write syntax.html )
 */
/*
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_BASENAME
# ifdef HAVE_LIBGEN_H
#  include <libgen.h>
# else
#  include <unistd.h>
# endif
#endif
#include <stdarg.h>

#include "mkdio.h"
#include "cstring.h"
#include "amalloc.h"

#include "gethopt.h"

char *pgm = "mkd2html";

extern int notspecial(char *filename);

#ifndef HAVE_BASENAME
char *
basename(char *path)
{
    char *p;

    if ( p = strrchr(path, '/') )
	return 1+p;
    return path;
}
#endif

void
fail(char *why, ...)
{
    va_list ptr;

    va_start(ptr,why);
    fprintf(stderr, "%s: ", pgm);
    vfprintf(stderr, why, ptr);
    fputc('\n', stderr);
    va_end(ptr);
    exit(1);
}


enum { ADD_CSS, ADD_HEADER, ADD_FOOTER };

struct h_opt opts[] = {
    { ADD_CSS,       "css", 0, "url",    "Additional css for this page" },
    { ADD_HEADER, "header", 0, "header", "Additonal headers for this page" },
    { ADD_FOOTER, "footer", 0, "footer", "Additional footers for this page" },
};
#define NROPTS (sizeof opts/sizeof opts[0])

#if USE_H1TITLE
extern char* mkd_h1_title(MMIOT *);
#endif


main(argc, argv)
char **argv;
{
    char *h;
    char *source = 0, *dest = 0;
    MMIOT *mmiot;
    int i;
    FILE *input, *output; 
    STRING(char*) css, headers, footers;
    struct h_opt *res;
    struct h_context flags;


    CREATE(css);
    CREATE(headers);
    CREATE(footers);
    pgm = basename(argv[0]);

    hoptset(&flags, argc, argv);
    hopterr(&flags, 1);
    while ( res = gethopt(&flags, opts, NROPTS) ) {
	if ( res == HOPTERR ) {
	    hoptusage(argv[0], opts, NROPTS, "source [dest]");
	    exit(1);
	}
	
	switch ( res->option ) {
	case ADD_CSS:
	    EXPAND(css) = hoptarg(&flags);
	    break;
	case ADD_HEADER:
	    EXPAND(headers) = hoptarg(&flags);
	    break;
	case ADD_FOOTER:
	    EXPAND(footers) = hoptarg(&flags);
	    break;
	default:
	    fprintf(stderr, "unknown option?\n");
	    break;
	}
    }

    argc -= hoptind(&flags);
    argv += hoptind(&flags);

    switch ( argc ) {
	char *p, *dot;
    case 0:
	input = stdin;
	output = stdout;
	break;
    
    case 1:
    case 2:
	dest   = malloc(strlen(argv[argc-1]) + 6);
	source = malloc(strlen(argv[0]) + 6);

	if ( !(source && dest) )
	    fail("out of memory allocating name buffers");

	strcpy(source, argv[0]);
	if (( p = strrchr(source, '/') ))
	    p = source;
	else
	    ++p;

	if ( (input = fopen(source, "r")) == 0 ) {
	    strcat(source, ".text");
	    if ( (input = fopen(source, "r")) == 0 )
		fail("can't open either %s or %s", argv[0], source);
	}
	strcpy(dest, source);

	if ( notspecial(dest) ) {
	    if (( dot = strrchr(dest, '.') ))
		*dot = 0;
	    strcat(dest, ".html");
	}

	if ( (output = fopen(dest, "w")) == 0 )
	    fail("can't write to %s", dest);
	break;

    default:
	hoptusage(argv[0], opts, NROPTS, "source [dest]");
	exit(1);
    }

    if ( (mmiot = mkd_in(input, 0)) == 0 )
	fail("can't read %s", source ? source : "stdin");

    if ( !mkd_compile(mmiot, 0) )
	fail("couldn't compile input");


    h = mkd_doc_title(mmiot);
#if USE_H1TITLE
    if ( ! h )
	h = mkd_h1_title(mmiot);
#endif

    /* print a header */

    fprintf(output,
	"<!doctype html public \"-//W3C//DTD HTML 4.0 Transitional //EN\">\n"
	"<html>\n"
	"<head>\n"
	"  <meta name=\"GENERATOR\" content=\"mkd2html %s\">\n", markdown_version);

    fprintf(output,"  <meta http-equiv=\"Content-Type\""
		          " content=\"text/html; charset=utf-8\">\n");

    for ( i=0; i < S(css); i++ )
	fprintf(output, "  <link rel=\"stylesheet\"\n"
			"        type=\"text/css\"\n"
			"        href=\"%s\" />\n", T(css)[i]);

    fprintf(output,"  <title>");
    if ( h )
	mkd_generateline(h, strlen(h), output, 0);
    /* xhtml requires a <title> in the header, even if it doesn't
     * contain anything
     */
    fprintf(output, "</title>\n");
    
    for ( i=0; i < S(headers); i++ )
	fprintf(output, "  %s\n", T(headers)[i]);
    fprintf(output, "</head>\n"
		    "<body>\n");

    /* print the compiled body */

    mkd_generatehtml(mmiot, output);

    for ( i=0; i < S(footers); i++ )
	fprintf(output, "%s\n", T(footers)[i]);
    
    fprintf(output, "</body>\n"
		    "</html>\n");
    
    mkd_cleanup(mmiot);
    exit(0);
}
