/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007-2011 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "markdown.h"
#include "amalloc.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

static void set_dlist(mkd_flag_t *, int);
static void set_standard(mkd_flag_t *, int);

static struct _special {
    char *name;
    void (*setter)(mkd_flag_t *, int);
} special[] = { 
    { "definitionlist", set_dlist },
    { "dlist",          set_dlist },
    { "standard",       set_standard },
};

static struct _opt {
    char *name;
    char *desc;
    int special;
    int off;
    int alias; 		/* this opt is a synonym; don't display in -F? */
    int sayenable;
    int flag;	/* flag to set/clear */
    int unflag;	/* flag to clear/set */
} opts[] = {
    { "tabstop",       "default (4-space) tabstops", 0, 0, 0, 1, MKD_TABSTOP  },
    { "image",         "images",                     0, 1, 0, 1, MKD_NOIMAGE  },
    { "links",         "links",                      0, 1, 0, 1, MKD_NOLINKS  },
    { "strict",        "emphasis inside words",      0, 0, 0, 1, MKD_NORELAXED  },
    { "relax",         "emphasis inside words",      0, 1, 1, 1, MKD_NORELAXED  },
    { "tables",        "tables",                     0, 1, 0, 1, MKD_NOTABLES },
    { "header",        "pandoc-style headers",       0, 1, 0, 1, MKD_NOHEADER },
    { "html",          "allow raw html",             0, 1, 0, 0, MKD_NOHTML   },
    { "ext",           "extended protocols",         0, 1, 0, 1, MKD_NO_EXT   },
    { "cdata",         "generate cdata",             0, 0, 0, 0, MKD_CDATA    },
    { "smarty",        "smartypants",                0, 1, 0, 1, MKD_NOPANTS  },
    { "pants",         "smartypants",                0, 1, 1, 1, MKD_NOPANTS  },
    { "toc",           "tables of contents",         0, 0, 0, 1, MKD_TOC      },
    { "autolink",      "autolinking",                0, 0, 0, 1, MKD_AUTOLINK },
    { "safelink",      "safe links",                 0, 0, 0, 1, MKD_SAFELINK },
    { "strikethrough", "strikethrough",              0, 1, 0, 1, MKD_NOSTRIKETHROUGH },
    { "del",           "strikethrough",              0, 1, 1, 1, MKD_NOSTRIKETHROUGH },
    { "superscript",   "superscript",                0, 1, 0, 1, MKD_NOSUPERSCRIPT },
    { "divquote",      ">%class% blockquotes",       0, 1, 0, 1, MKD_NODIVQUOTE },
    { "alphalist",     "alpha lists",                0, 1, 0, 1, MKD_NOALPHALIST },
    { "1.0",           "markdown 1.0 compatibility", 0, 0, 0, 1, MKD_1_COMPAT },
    { "footnotes",     "markdown extra footnotes",   0, 0, 0, 1, MKD_EXTRA_FOOTNOTE },
    { "footnote",      "markdown extra footnotes",   0, 0, 1, 1, MKD_EXTRA_FOOTNOTE },
    { "style",         "extract style blocks",       0, 1, 0, 1, MKD_NOSTYLE },
    { "dldiscount",    "discount-style definition lists", 0, 0, 0, 1, MKD_DLDISCOUNT },
    { "dlextra",       "markdown extra-style definition lists", 0, 0, 0, 1, MKD_DLEXTRA },
    { "fencedcode",    "fenced code blocks",         0, 0, 0, 1, MKD_FENCEDCODE },
    { "idanchor",      "id= anchors in TOC",         0, 0, 0, 1, MKD_IDANCHOR },
    { "githubtags",    "- and _ in element names",   0, 0, 0, 1, MKD_GITHUBTAGS },
    { "urlencodedanchor", "html5-style anchors",     0, 0, 0, 1, MKD_URLENCODEDANCHOR },
    { "html5anchor",   "html5-style anchors",        0, 0, 1, 1, MKD_URLENCODEDANCHOR },
    { "latex",         "LaTeX escapes",              0, 0, 0, 1, MKD_LATEX },
    { "explicitlist",  "merge adjacent numeric/bullet lists", 0, 0, 0, 0, MKD_EXPLICITLIST },
    { "github-listitem","github-style check items",  0, 1, 0, 1, MKD_NORMAL_LISTITEM } ,
    { "regular-listitem","github-style check items", 0, 0, 1, 1, MKD_NORMAL_LISTITEM } ,
    { "definitionlist","both discount & markdown extra definition lists", 1 },
    { "dlist",         "both discount & markdown extra definition lists", 1, 0, 1 },
    { "standard",      "conform to the markdown standard", 1 },
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
show_flags(int byname, int verbose, mkd_flag_t *flags)
{
    int i;

    if ( byname ) {
	qsort(opts, NR(opts), sizeof(opts[0]), (stfu)sort_by_name);
    
	for (i=0; i < NR(opts); i++) {
	    if ( opts[i].alias && !verbose )
		continue;
	    if ( (flags==0) || is_flag_set(flags, opts[i].flag) )
		fprintf(stderr, "%16s : %s\n", opts[i].name, opts[i].desc);
	}
    }
    else {
	qsort(opts, NR(opts), sizeof(opts[0]), (stfu)sort_by_flag);
	
	for (i=0; i < NR(opts) && i < 8*sizeof(DWORD); i++) {
	    
	    if ( opts[i].special || opts[i].alias )
		continue;

	    if ( (flags==0) || is_flag_set(flags, opts[i].flag) ) {
		fprintf(stderr, "%08lx : ", 1L<<opts[i].flag);
		if ( opts[i].sayenable )
		    fprintf(stderr, opts[i].off ? "disable " : "enable ");
		fprintf(stderr, "%s\n", opts[i].desc);
	    }
	}
    }
}

static void
set_dlist(mkd_flag_t *flags, int enable)
{
    if ( enable ) {
	set_mkd_flag(flags, MKD_DLDISCOUNT);
	set_mkd_flag(flags, MKD_DLEXTRA);
    }
    else {
	clear_mkd_flag(flags, MKD_DLDISCOUNT);
	clear_mkd_flag(flags, MKD_DLEXTRA);
    }
}


static void
set_standard(mkd_flag_t *flags, int enable)
{
    if ( enable ) {
	clear_mkd_flag(flags, MKD_FENCEDCODE);
	clear_mkd_flag(flags, MKD_LATEX);
	clear_mkd_flag(flags, MKD_TABSTOP);
	clear_mkd_flag(flags, MKD_EXTRA_FOOTNOTE);
	clear_mkd_flag(flags, MKD_AUTOLINK);
	clear_mkd_flag(flags, MKD_SAFELINK);
	clear_mkd_flag(flags, MKD_TOC);
	clear_mkd_flag(flags, MKD_DLDISCOUNT);
	clear_mkd_flag(flags, MKD_DLEXTRA);
	set_mkd_flag(flags,   MKD_NOSUPERSCRIPT);
	set_mkd_flag(flags,   MKD_NORMAL_LISTITEM);
	set_mkd_flag(flags,   MKD_NO_EXT);
	set_mkd_flag(flags,   MKD_NOSUPERSCRIPT);
	set_mkd_flag(flags,   MKD_NORELAXED);
	set_mkd_flag(flags,   MKD_NOTABLES);
	set_mkd_flag(flags,   MKD_NOSTRIKETHROUGH);
	set_mkd_flag(flags,   MKD_NOHEADER);
	set_mkd_flag(flags,   MKD_NODIVQUOTE);
	set_mkd_flag(flags,   MKD_NOALPHALIST);
	set_mkd_flag(flags,   MKD_NOSTYLE);
    }
}
    

static void
handle_special(mkd_flag_t *flags, char *opt, int enable)
{
    int i;

    for (i=0; i < NR(special); i++)
	if ( strcasecmp(opt, special[i].name) == 0 ) {
	    (special[i].setter)(flags, enable);
	    return;
	}
}

char *
mkd_set_flag_string(mkd_flag_t *flags, char *optionstring)
{
    int i;
    int enable;
    char *arg;

    if ( flags == 0 )	/* shouldn't happen */
	return "NULL";

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

	    if ( opts[i].special ) {
		handle_special(flags, opts[i].name, enable);
		continue;
	    }
	    if ( opts[i].off )
		enable = !enable;

	    if ( enable ) {
		set_mkd_flag(flags, opts[i].flag);
		if ( opts[i].unflag )
		    clear_mkd_flag(flags, opts[i].unflag);
	    }
	    else {
		clear_mkd_flag(flags, opts[i].flag);
		if ( opts[i].unflag )
		    set_mkd_flag(flags, opts[i].unflag);
	    }
	}
	else
	    return arg;
    }
    return 0;
}


void
mkd_set_flag_num(mkd_flag_t *p, unsigned long bit)
{
    if ( p && (bit < MKD_NR_FLAGS) )
	set_mkd_flag(p, bit);
}


void
mkd_clr_flag_num(mkd_flag_t *p, unsigned long bit)
{
    if ( p && (bit < MKD_NR_FLAGS) )
	clear_mkd_flag(p, bit);
}


void
mkd_set_flag_bitmap(mkd_flag_t *p, long bits)
{
    int i;
    
    if ( p == 0 )
	return;

    for (i=0; i < 8*sizeof(long) && i < MKD_NR_FLAGS; i++)
	if ( bits & (1<<i) )
	    set_mkd_flag(p, i);
}
