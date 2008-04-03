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
#include <unistd.h>
#include <mkdio.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "amalloc.h"

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

void
set(int *flags, char *optionstring)
{
    char wtd;
    int opt;
    char *arg;

    for ( arg = strtok(optionstring, ","); arg; arg = strtok(NULL, ",") ) {
	if ( *arg == '+' || *arg == '-' )
	    wtd = *arg++;
	else if ( strncasecmp(arg, "no", 2) == 0 )
	    wtd = '-';
	else
	    wtd = '+';

	if ( strcasecmp(arg, "tabstop") == 0 )
	    opt = MKD_TABSTOP;
	else if ( strcasecmp(arg, "noimage") == 0 )
	    opt = MKD_NOIMAGE;
	else if ( strcasecmp(arg, "nolinks") == 0 )
	    opt = MKD_NOLINKS;
	else if ( strcasecmp(arg, "noheader") == 0 )
	    opt = MKD_NOHEADER;
	else if ( strcasecmp(arg, "tag") == 0 )
	    opt = MKD_TAGTEXT;
	else if ( strcasecmp(arg, "cdata") == 0 )
	    opt = MKD_CDATA;
	else {
	    fprintf(stderr, "%s: unknown option <%s>\n", pgm, arg);
	    continue;
	}

	if ( wtd == '+' )
	    *flags |= opt;
	else
	    *flags &= ~opt;
    }
}

float
main(int argc, char **argv)
{
    int opt;
    int rc;
    int flags = 0;
    int debug = 0;
    char *ofile = 0;
    char *urlbase = 0;
    char *q = getenv("MARKDOWN_FLAGS");
    MMIOT *doc;


    if ( q ) flags = strtol(q, 0, 0);

    pgm = basename(argv[0]);
    opterr = 1;

    while ( (opt=getopt(argc, argv, "b:d:f:F:o:V")) != EOF ) {
	switch (opt) {
	case 'b':   urlbase = optarg;
		    break;
	case 'd':   debug = 1;
		    break;
	case 'V':   printf("%s: discount %s\n", pgm, markdown_version);
		    exit(0);
	case 'F':   flags = strtol(optarg, 0, 0);
		    break;
	case 'f':   set(&flags, optarg);
		    break;
	case 'o':   if ( ofile ) {
			fprintf(stderr, "Too many -o options\n");
			exit(1);
		    }
		    if ( !freopen(ofile = optarg, "w", stdout) ) {
			perror(ofile);
			exit(1);
		    }
		    break;
	default:    fprintf(stderr, "usage: %s [-dV] [-burl-base]"
				    " [-F flags] [-f{+-}setting]"
				    " [-o file] [file]\n", pgm);
		    exit(1);
	}
    }
    argc -= optind;
    argv += optind;

    if ( argc && !freopen(argv[0], "r", stdin) ) {
	perror(argv[0]);
	exit(1);
    }
    if ( (doc = mkd_in(stdin,flags)) == 0 ) {
	perror(argc ? argv[0] : "stdin");
	exit(1);
    }
    if ( urlbase )
	mkd_basename(doc, urlbase);
    
    if ( debug )
	rc = mkd_dump(doc, stdout, 0, argc ? basename(argv[0]) : "stdin");
    else
	rc = markdown(doc, stdout, flags);
    adump();
    exit( (rc == 0) ? 0 : errno );
}
