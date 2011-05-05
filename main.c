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

static struct _opt {
    char *name;
    char *desc;
    int off;
    int skip;
    int sayenable;
    mkd_flag_t flag;
} opts[] = {
    { "tabstop",       "default (4-space) tabstops", 0, 0, 1, MKD_TABSTOP  },
    { "image",         "images",                     1, 0, 1, MKD_NOIMAGE  },
    { "links",         "links",                      1, 0, 1, MKD_NOLINKS  },
    { "relax",         "emphasis inside words",      1, 1, 1, MKD_STRICT   },
    { "strict",        "emphasis inside words",      0, 0, 1, MKD_STRICT   },
    { "tables",        "tables",                     1, 0, 1, MKD_NOTABLES },
    { "header",        "pandoc-style headers",       1, 0, 1, MKD_NOHEADER },
    { "html",          "raw html",                   1, 0, 1, MKD_NOHTML   },
    { "ext",           "extended protocols",         1, 0, 1, MKD_NO_EXT   },
    { "cdata",         "generate cdata",             0, 0, 0, MKD_CDATA    },
    { "smarty",        "smartypants",                1, 0, 1, MKD_NOPANTS  },
    { "pants",         "smartypants",                1, 1, 1, MKD_NOPANTS  },
    { "toc",           "tables of contents",         0, 0, 1, MKD_TOC      },
    { "autolink",      "autolinking",                0, 0, 1, MKD_AUTOLINK },
    { "safelink",      "safe links",                 0, 0, 1, MKD_SAFELINK },
    { "strikethrough", "strikethrough",              1, 0, 1, MKD_NOSTRIKETHROUGH },
    { "del",           "strikethrough",              1, 1, 1, MKD_NOSTRIKETHROUGH },
    { "superscript",   "superscript",                1, 0, 1, MKD_NOSUPERSCRIPT },
    { "emphasis",      "emphasis inside words",      0, 0, 1, MKD_NORELAXED },
    { "divquote",      ">%class% blockquotes",       1, 0, 1, MKD_NODIVQUOTE },
    { "alphalist",     "alpha lists",                1, 0, 1, MKD_NOALPHALIST },
    { "definitionlist","definition lists",           1, 0, 1, MKD_NODLIST },
    { "1.0",           "markdown 1.0 compatibility", 0, 0, 1, MKD_1_COMPAT },
    { "footnotes",     "markdown extra footnotes",   0, 0, 1, MKD_EXTRA_FOOTNOTE },
    { "footnote",      "markdown extra footnotes",   0, 1, 1, MKD_EXTRA_FOOTNOTE },
} ;

#define NR(x)	(sizeof x / sizeof x[0])


int
sort_by_name(struct _opt *a, struct _opt *b)
{
    return strcmp(a->name,b->name);
}

int
sort_by_flag(struct _opt *a, struct _opt *b)
{
    return a->flag - b->flag;
}


void
show_flags(int byname)
{
    int i;

    if ( byname ) {
	qsort(opts, NR(opts), sizeof(opts[0]), sort_by_name);
    
	for (i=0; i < NR(opts); i++)
	    if ( ! opts[i].skip )
		fprintf(stderr, "%16s : %s\n", opts[i].name, opts[i].desc);
    }
    else {
	qsort(opts, NR(opts), sizeof(opts[0]), sort_by_flag);
	
	for (i=0; i < NR(opts); i++)
	    if ( ! opts[i].skip ) {
		fprintf(stderr, "%08lx : ", (long)opts[i].flag);
		if ( opts[i].sayenable )
		    fprintf(stderr, opts[i].off ? "disable " : "enable ");
		fprintf(stderr, "%s\n", opts[i].desc);
	    }
    }
}
    

void
set(mkd_flag_t *flags, char *optionstring)
{
    int i;
    int enable;
    char *arg;

    for ( arg = strtok(optionstring, ","); arg; arg = strtok(NULL, ",") ) {
	if ( *arg == '+' || *arg == '-' )
	    enable = (*arg++ == '+') ? 1 : 0;
	else if ( strncasecmp(arg, "no", 2) == 0 ) {
	    arg += 2;
	    enable = 0;
	}
	else
	    enable = 1;

	for ( i=0; i < NR(opts); i++ )
	    if ( strcasecmp(arg, opts[i].name) == 0 )
		break;

	if ( i < NR(opts) ) {
	    if ( opts[i].off )
		enable = !enable;
		
	    if ( enable )
		*flags |= opts[i].flag;
	    else
		*flags &= ~opts[i].flag;
	}
	else
	    fprintf(stderr, "%s: unknown option <%s>\n", pgm, arg);
    }
}


char *
e_flags(const char *text, const int size, void *context)
{
    return (char*)context;
}


float
main(int argc, char **argv)
{
    int opt;
    int rc;
    mkd_flag_t flags = 0;
    int debug = 0;
    int toc = 0;
    int version = 0;
    int with_html5 = 0;
    int use_mkd_line = 0;
    char *extra_footnote_prefix = 0;
    char *urlflags = 0;
    char *text = 0;
    char *ofile = 0;
    char *urlbase = 0;
    char *q;
    MMIOT *doc;

    if ( q = getenv("MARKDOWN_FLAGS") )
	flags = strtol(q, 0, 0);

    pgm = basename(argv[0]);
    opterr = 1;

    while ( (opt=getopt(argc, argv, "5b:C:df:E:F:o:s:t:TV")) != EOF ) {
	switch (opt) {
	case '5':   with_html5 = 1;
		    break;
	case 'b':   urlbase = optarg;
		    break;
	case 'd':   debug = 1;
		    break;
	case 'V':   version++;
		    break;
	case 'E':   urlflags = optarg;
		    break;
	case 'F':   if ( strcmp(optarg, "?") == 0 ) {
			show_flags(0);
			exit(0);
		    }
		    else
			flags = strtol(optarg, 0, 0);
		    break;
	case 'f':   if ( strcmp(optarg, "?") == 0 ) {
			show_flags(1);
			exit(0);
		    }
		    else
			set(&flags, optarg);
		    break;
	case 't':   text = optarg;
		    use_mkd_line = 1;
		    break;
	case 'T':   toc = 1;
		    break;
	case 's':   text = optarg;
		    break;
	case 'C':   extra_footnote_prefix = optarg;
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
	default:    fprintf(stderr, "usage: %s [-dTV] [-b url-base]"
				    " [-F bitmap] [-f {+-}flags]"
				    " [-o ofile] [-s text]"
				    " [-t text] [file]\n", pgm);
		    exit(1);
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

    argc -= optind;
    argv += optind;

    if ( with_html5 )
	mkd_with_html5_tags();

    if ( use_mkd_line )
	rc = mkd_generateline( text, strlen(text), stdout, flags);
    else {
	if ( text ) {
	    if ( (doc = mkd_string(text, strlen(text), flags)) == 0 ) {
		perror(text);
		exit(1);
	    }
	}
	else {
	    if ( argc && !freopen(argv[0], "r", stdin) ) {
		perror(argv[0]);
		exit(1);
	    }
	    if ( (doc = mkd_in(stdin,flags)) == 0 ) {
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
		if ( toc )
		    mkd_generatetoc(doc, stdout);
		mkd_generatehtml(doc, stdout);
		mkd_cleanup(doc);
	    }
	}
    }
    mkd_deallocate_tags();
    adump();
    exit( (rc == 0) ? 0 : errno );
}
