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
	wtd = '-';
	if ( *arg == '+' || *arg == '-' )
	    wtd = *arg++;

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
    char *q = getenv("MARKDOWN_FLAGS");


    if ( q ) flags = strtol(q, 0, 0);

    pgm = basename(argv[0]);
    opterr = 1;

    while ( (opt=getopt(argc, argv, "d:f:F:o:V")) != EOF ) {
	switch (opt) {
	case 'd':   debug = 1;
		    break;
	case 'V':   printf("%s: discount %s\n", pgm, version);
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
	default:    fprintf(stderr, "usage: %s [-dV]"
				    " [-F flags] [-f{+-}setting"
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
    if ( debug )
	rc = mkd_dump(mkd_in(stdin, flags), stdout, 0,
		      argc ? basename(argv[0]) : "stdin");
    else
	rc = markdown(mkd_in(stdin, flags), stdout, flags);
    exit( (rc == 0) ? 0 : errno );
}
