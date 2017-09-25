/*
 * markdown: convert a single markdown document into html
 */
/*
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mkdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "amalloc.h"
#include "pgm_options.h"
#include "tags.h"
#include "gethopt.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifndef HAVE_BASENAME
#include <string.h>

char*
basename(char *p)
{
    char *ret = strrchr(p, '/');

    return ret ? (1+ret) : p;
}
#endif


char *pgm = "markdown";

char *
e_flags(const char *text, const int size, void *context)
{
    return (char*)context;
}


void
complain(char *fmt, ...)
{
    va_list ptr;

    fprintf(stderr, "%s: ", pgm);
    va_start(ptr, fmt);
    vfprintf(stderr, fmt, ptr);
    va_end(ptr);
    fputc('\n', stderr);
    fflush(stderr);
}


struct h_opt opts[] = {
    { 0, "html5",  '5', 0,          "recognise html5 block elements" },
    { 0, "base",   'b', "url-base", "URL prefix" },
    { 0, "debug",  'd', 0,           "debugging" },
    { 0, "version",'V', 0,           "show version info" },
    { 0, 0,        'E', "flags",     "url flags" },
    { 0, 0,        'F', "bitmap",    "set/show hex flags" },
    { 0, 0,        'f', "{+-}flags", "set/show named flags" },
    { 0, 0,        'G', 0,           "github flavoured markdown" },
    { 0, 0,        'n', 0,           "don't write generated html" },
    { 0, 0,        's', "text",      "format `text`" },
    { 0, "style",  'S', 0,           "output <style> blocks" },
    { 0, 0,        't', "text",      "format `text` with mkd_line()" },
    { 0, "toc",    'T', 0,           "output a TOC" },
    { 0, 0,        'C', "prefix",    "prefix for markdown extra footnotes" },
    { 0, 0,        'o', "file",       "write output to file" },
};
#define NROPTS (sizeof opts/sizeof opts[0])

int
main(int argc, char **argv)
{
    int rc;
    mkd_flag_t flags = 0;
    int debug = 0;
    int toc = 0;
    int content = 1;
    int version = 0;
    int with_html5 = 0;
    int styles = 0;
    int use_mkd_line = 0;
    int github_flavoured = 0;
    char *extra_footnote_prefix = 0;
    char *urlflags = 0;
    char *text = 0;
    char *ofile = 0;
    char *urlbase = 0;
    char *q;
    MMIOT *doc;
    struct h_context blob;
    struct h_opt *opt;

    hoptset(&blob, argc, argv);
    hopterr(&blob, 1);

    if ( q = getenv("MARKDOWN_FLAGS") )
	flags = strtol(q, 0, 0);

    pgm = basename(argv[0]);

    while ( opt=gethopt(&blob, opts, NROPTS) ) {
	if ( opt == HOPTERR ) {
	    hoptusage(pgm, opts, NROPTS, "[file]");
	    exit(1);
	}
	switch (opt->optchar) {
	case '5':   with_html5 = 1;
		    break;
	case 'b':   urlbase = hoptarg(&blob);
		    break;
	case 'd':   debug = 1;
		    break;
	case 'V':   version++;
		    break;
	case 'E':   urlflags = hoptarg(&blob);
		    break;
	case 'F':   if ( strcmp(hoptarg(&blob), "?") == 0 ) {
			show_flags(0, 0);
			exit(0);
		    }
		    else
			flags = strtol(hoptarg(&blob), 0, 0);
		    break;
	case 'f':   if ( strcmp(hoptarg(&blob), "?") == 0 ) {
			show_flags(1, version);
			exit(0);
		    }
		    else if ( q=set_flag(&flags, hoptarg(&blob)) )
			complain("unknown option <%s>", q);
		    break;
	case 'G':   github_flavoured = 1;
		    break;
	case 'n':   content = 0;
		    break;
	case 's':   text = hoptarg(&blob);
		    break;
	case 'S':   styles = 1;
		    break;
	case 't':   text = hoptarg(&blob);
		    use_mkd_line = 1;
		    break;
	case 'T':   toc = 1;
		    break;
	case 'C':   extra_footnote_prefix = hoptarg(&blob);
		    break;
	case 'o':   if ( ofile ) {
			complain("Too many -o options");
			exit(1);
		    }
		    if ( !freopen(ofile = hoptarg(&blob), "w", stdout) ) {
			perror(ofile);
			exit(1);
		    }
		    break;
	}
    }

    if ( version ) {
	printf("%s: discount %s%s", pgm, markdown_version,
				  with_html5 ? " +html5":"");
	if ( version > 1 )
	    mkd_flags_are(stdout, flags, 0);
	putchar('\n');
	exit(0);
    }

    argc -= hoptind(&blob);
    argv += hoptind(&blob);

    if ( with_html5 )
	mkd_with_html5_tags();

    if ( use_mkd_line )
	rc = mkd_generateline( text, strlen(text), stdout, flags);
    else {
	if ( text ) {
	    doc = github_flavoured ? gfm_string(text, strlen(text), flags)
				   : mkd_string(text, strlen(text), flags) ;

	    if ( !doc ) {
		perror(text);
		exit(1);
	    }
	}
	else {
	    if ( argc && !freopen(argv[0], "r", stdin) ) {
		perror(argv[0]);
		exit(1);
	    }

	    doc = github_flavoured ? gfm_in(stdin,flags) : mkd_in(stdin,flags);
	    if ( !doc ) {
		perror(argc ? argv[0] : "stdin");
		exit(1);
	    }
	}
	if ( urlbase )
	    mkd_basename(doc, urlbase);
	if ( urlflags ) {
	    mkd_e_data(doc, urlflags);
	    mkd_e_flags(doc, e_flags);
	}
	if ( extra_footnote_prefix )
	    mkd_ref_prefix(doc, extra_footnote_prefix);

	if ( debug )
	    rc = mkd_dump(doc, stdout, 0, argc ? basename(argv[0]) : "stdin");
	else {
	    rc = 1;
	    if ( mkd_compile(doc, flags) ) {
		rc = 0;
		if ( styles )
		    mkd_generatecss(doc, stdout);
		if ( toc )
		    mkd_generatetoc(doc, stdout);
		if ( content )
		    mkd_generatehtml(doc, stdout);
	    }
	}
	mkd_cleanup(doc);
    }
    mkd_deallocate_tags();
    adump();
    exit( (rc == 0) ? 0 : errno );
}
