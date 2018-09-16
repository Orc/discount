/*
 * Copyright (C) 2018 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"
#include "pgm_options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#if HAVE_PWD_H
#  include <pwd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "mkdio.h"
#include "cstring.h"
#include "amalloc.h"
#include "gethopt.h"

char *pgm = "pandoc headers";

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


struct h_opt opts[] = {
    { 0, "author", 'a', 0, "show the author line" },
    { 0, "title" , 't', 0, "show the title line" },
    { 0, "date"  , 'd', 0, "show the date line" },
} ;
#define NROPTS (sizeof opts / sizeof opts[0])

int
main(argc, argv)
char **argv;
{
    int show_author=0, show_title=0, show_date=0;
    MMIOT *p;
    char *res;
    struct h_opt *opt;
    struct h_context blob;
    
    hoptset(&blob, argc, argv);
    
    while ( opt = gethopt(&blob, opts, NROPTS) ) {
	if ( opt && (opt != HOPTERR) ) {
	    switch ( opt->optchar ) {
	    case 'a':   show_author = 1;
			break;
	    case 't':   show_title = 1;
			break;
	    case 'd':   show_date = 1;
			break;
	    }
	}
    }

    p = mkd_in(stdin, 0);

    if ( p == 0 )
	fail("could not read input?");

    if ( !mkd_compile(p, 0) )
	fail("could not compile input?");
	
    if (show_author) {
	if ( res = mkd_doc_author(p) )
	    printf("author: %s\n", res);
    }
    if (show_title) {
	if ( res = mkd_doc_title(p) )
	    printf("title : %s\n", res);
    }
    if (show_date) {
	if ( res = mkd_doc_date(p) )
	    printf("date   : %s\n", res);
    }
    mkd_cleanup(p);
    exit(0);
}
