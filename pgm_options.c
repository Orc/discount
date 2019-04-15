/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007-2011 David L Parsons.
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

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

static struct _opt {
    char *name;
    char *desc;
    int off;
    int skip; 		/* this opt is a synonym; don't display in -F? */
    int composite;	/* composite flag; don't display in -F? */
    int sayenable;
    mkd_flag_t flag;	/* flags to set/clear */
    mkd_flag_t unflag;	/* flags to clear/set */
} opts[] = {
    { "tabstop",       "default (4-space) tabstops", 0, 0, 0, 1, MKD_TABSTOP  },
    { "image",         "images",                     1, 0, 0, 1, MKD_NOIMAGE  },
    { "links",         "links",                      1, 0, 0, 1, MKD_NOLINKS  },
    { "strict",        "emphasis inside words",      0, 0, 0, 1, MKD_NORELAXED  },
    { "relax",         "emphasis inside words",      1, 1, 0, 1, MKD_NORELAXED  },
    { "tables",        "tables",                     1, 0, 0, 1, MKD_NOTABLES },
    { "header",        "pandoc-style headers",       1, 0, 0, 1, MKD_NOHEADER },
    { "html",          "allow raw html",             1, 0, 0, 0, MKD_NOHTML   },
    { "ext",           "extended protocols",         1, 0, 0, 1, MKD_NO_EXT   },
    { "cdata",         "generate cdata",             0, 0, 0, 0, MKD_CDATA    },
    { "smarty",        "smartypants",                1, 0, 0, 1, MKD_NOPANTS  },
    { "pants",         "smartypants",                1, 1, 0, 1, MKD_NOPANTS  },
    { "toc",           "tables of contents",         0, 0, 0, 1, MKD_TOC      },
    { "autolink",      "autolinking",                0, 0, 0, 1, MKD_AUTOLINK },
    { "safelink",      "safe links",                 0, 0, 0, 1, MKD_SAFELINK },
    { "strikethrough", "strikethrough",              1, 0, 0, 1, MKD_NOSTRIKETHROUGH },
    { "del",           "strikethrough",              1, 1, 0, 1, MKD_NOSTRIKETHROUGH },
    { "superscript",   "superscript",                1, 0, 0, 1, MKD_NOSUPERSCRIPT },
    { "divquote",      ">%class% blockquotes",       1, 0, 0, 1, MKD_NODIVQUOTE },
    { "alphalist",     "alpha lists",                1, 0, 0, 1, MKD_NOALPHALIST },
    { "definitionlist","both discount & markdown extra definition lists", 0, 0, 1, 1, MKD_DLIST },
    { "dlist",         "both discount & markdown extra definition lists", 0, 1, 1, 1, MKD_DLIST },
    { "1.0",           "markdown 1.0 compatibility", 0, 0, 0, 1, MKD_1_COMPAT },
    { "footnotes",     "markdown extra footnotes",   0, 0, 0, 1, MKD_EXTRA_FOOTNOTE },
    { "footnote",      "markdown extra footnotes",   0, 1, 0, 1, MKD_EXTRA_FOOTNOTE },
    { "style",         "extract style blocks",       1, 0, 0, 1, MKD_NOSTYLE },
    { "dldiscount",    "discount-style definition lists", 0, 0, 0, 1, MKD_DLDISCOUNT },
    { "dlextra",       "markdown extra-style definition lists", 0, 0, 0, 1, MKD_DLEXTRA },
    { "fencedcode",    "fenced code blocks",         0, 0, 0, 1, MKD_FENCEDCODE },
    { "idanchor",      "id= anchors in TOC",         0, 0, 0, 1, MKD_IDANCHOR },
    { "githubtags",    "- and _ in element names",   0, 0, 0, 1, MKD_GITHUBTAGS },
    { "urlencodedanchor", "html5-style anchors",     0, 0, 0, 1, MKD_URLENCODEDANCHOR },
    { "html5anchor",   "html5-style anchors",        0, 1, 0, 1, MKD_URLENCODEDANCHOR },
    { "latex",         "LaTeX escapes",              0, 0, 0, 1, MKD_LATEX },
    { "explicitlist",  "merge adjacent numeric/bullet lists", 0, 0, 0, 0, MKD_EXPLICITLIST },
    { "github-listitem","github-style check items",  1, 0, 0, 1, MKD_NORMAL_LISTITEM } ,
    { "regular-listitem","github-style check items", 0, 1, 0, 1, MKD_NORMAL_LISTITEM } ,
    { "standard",      "conform to the markdown standard", 0, 0, 1, 0, MKD_STRICT,
	 MKD_FENCEDCODE|MKD_LATEX|MKD_TABSTOP|MKD_EXTRA_FOOTNOTE|MKD_AUTOLINK|MKD_SAFELINK|MKD_TOC|MKD_DLIST },
} ;

#define NR(x)	(sizeof x / sizeof x[0])


typedef int (*stfu)(const void *, const void *);

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
show_flags(int byname, int verbose)
{
    int i;

    if ( byname ) {
	qsort(opts, NR(opts), sizeof(opts[0]), (stfu)sort_by_name);
    
	for (i=0; i < NR(opts); i++)
	    if ( verbose || !opts[i].skip )
		fprintf(stderr, "%16s : %s\n", opts[i].name, opts[i].desc);
    }
    else {
	qsort(opts, NR(opts), sizeof(opts[0]), (stfu)sort_by_flag);
	
	for (i=0; i < NR(opts); i++)
	    if ( !(opts[i].skip || opts[i].composite) ) {
		fprintf(stderr, "%08lx : ", (long)opts[i].flag);
		if ( opts[i].sayenable )
		    fprintf(stderr, opts[i].off ? "disable " : "enable ");
		fprintf(stderr, "%s\n", opts[i].desc);
	    }
    }
}
    

char *
set_flag(mkd_flag_t *flags, char *optionstring)
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
		
	    if ( enable ) {
		*flags |= opts[i].flag;
		if ( opts[i].unflag )
		    *flags &= ~opts[i].unflag;
	    }
	    else {
		*flags &= ~opts[i].flag;
		if ( opts[i].unflag )
		    *flags |= opts[i].unflag;
	    }
	}
	else
	    return arg;
    }
    return 0;
}
