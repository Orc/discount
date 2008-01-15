/*
 * theme:  use a template to create a webpage (markdown-style)
 *
 * usage:  theme [-d root] [-t template] [-o html] [source]
 *
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
#if defined(HAVE_BASENAME) && defined(HAVE_LIBGEN_H)
#  include <libgen.h>
#endif
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "mkdio.h"
#include "cstring.h"

char *pgm = "theme";
char *output = 0;
char *source = 0;
char *root = 0;
struct passwd *me = 0;

#ifndef HAVE_BASENAME
char *
basename(char *path)
{
    char *p;

    if (( p = strrchr(path, '/') ))
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


/* open_template() -- start at the current directory and work up,
 *                    looking for the deepest nested template. 
 *                    Stop looking when we reach $root or /
 */
FILE *
open_template(char *template)
{
    char *cwd;
    int szcwd;
    int here = open(".", O_RDONLY);
    FILE *ret;

    if ( here == -1 )
	fail("cannot access the current directory");

    szcwd = root ? 1 + strlen(root) : 2;

    cwd = alloca(szcwd);

    while ( !(ret = fopen(template, "r")) ) {
	if ( getcwd(cwd, szcwd) == 0 ) {
	    if ( errno == ERANGE )
		goto up;
	    break;
	}

	if ( root && (strcmp(root, cwd) == 0) )
	    break;	/* ran out of paths to search */
	else if ( (strcmp(cwd, "/") == 0) || (*cwd == 0) )
	    break;	/* reached / */

    up: if ( chdir("..") == -1 )
	    break;
    }
    fchdir(here);
    close(here);
    return ret;
} /* open_template */


static STRING(int) pattern;
static int psp;

static int
prepare(FILE *input)
{
    int c;

    CREATE(pattern);
    psp = 0;
    while ( (c = getc(input)) != EOF )
	EXPAND(pattern) = c;
    fclose(input);
    return 1;
}

static int
pull()
{
    if ( psp < S(pattern) )
	return T(pattern)[psp++];
    return EOF;
}

static int
peek(int offset)
{
    int pos = (psp + offset)-1;

    if ( pos >= 0 && pos < S(pattern) )
	return T(pattern)[pos];

    return EOF;
}

static int
shift(int shiftwidth)
{
    psp += shiftwidth;
    return psp;
}

static int*
cursor()
{
    return T(pattern) + psp;
}


static int
thesame(int *p, char *pat)
{
    int i;

    for ( i=0; pat[i]; i++ ) {
	if ( pat[i] == ' ' ) {
	    if ( !isspace(peek(i+1)) ) {
		return 0;
	    }
	}
	else if ( tolower(peek(i+1)) != pat[i] ) {
	    return 0;
	}
    }
    return 1;
}


static int
istag(int *p, char *pat)
{
    int c;

    if ( thesame(p, pat) ) {
	c = peek(strlen(pat)+1);
	return (c == '>' || isspace(c));
    }
    return 0;
}


/* spin() - run through the theme template, looking for <?theme expansions
 *
 * theme expansions we love:
 *   <?theme date?>	-- the document date (from header)
 *   <?theme title?>	-- the document title (from header)
 *   <?theme author?>	-- the document author (from header)
 *   <?theme version?>  -- the version#
 *   <?theme body?>	-- the document body
 */
void
spin(FILE *template, MMIOT doc, FILE *output)
{
    int c;
    int *p;
    char *h;
    int flags;
    int intag=0, inhead=0;

    prepare(template);

    while ( (c = pull()) != EOF ) {
	if ( c == '<' ) {
	    if ( peek(1) == '!' && peek(2) == '-' && peek(3) == '-' ) {
		fputs("<!--", output);
		shift(3);
		do {
		    putc(c, output);
		} while ( ! (c == '-' && peek(1) == '-' && peek(2) == '>') );
	    }
	    else if ( (peek(1) == '?') && thesame(cursor(), "?theme ") ) {
		shift(strlen("?theme "));

		while ( ((c = pull()) != EOF) && isspace(c) )
		    ;

		shift(-1);
		p = cursor();

		if ( intag ) 
		    flags = MKD_EMBED;
		else if ( inhead )
		    flags = MKD_NOIMAGE|MKD_NOLINKS;
		else
		    flags = 0;

		if ( thesame(p, "title?>") ) {
		    if ( (h = mkd_doc_title(doc)) == 0 && source )
			h = source;

		    if ( h )
			mkd_text(h, strlen(h), output, flags);
		}
		else if ( thesame(p, "date?>") ) {
		    if (( h = mkd_doc_date(doc) ))
			mkd_text(h, strlen(h), output, flags);
		}
		else if ( thesame(p, "author?>") ) {
		    if ( (h = mkd_doc_author(doc)) == 0 && me )
			h = me->pw_gecos;

		    if ( h )
			mkd_text(h, strlen(h), output, flags);
		}
		else if ( thesame(p, "version?>") ) {
		    fwrite(version, strlen(version), 1, output);
		}
		else if ( thesame(p, "body?>") ) {
		    if ( !(inhead||intag) )
		    mkd_generatehtml(doc,output);
		}

		while ( (c = pull()) != EOF && (c != '?' && peek(1) != '>') )
		    ;
		shift(1);
	    }
	    else
		putc(c, output);

	    if ( istag(cursor(), "head") )
		inhead=1;
	    else if ( istag(cursor(), "body") )
		inhead=0;
	    intag=1;
	    continue;
	}
	else if ( c == '>' )
	    intag=0;

	putc(c, output);
    }
} /* spin */


void
main(argc, argv)
char **argv;
{
    char *template = "page.theme";
    FILE *tmplfile;
    int opt;
    MMIOT doc;

    if (( me = getpwuid(getuid()) )) {
	root = strdup(me->pw_dir);
	if ( !root )
	    fail("out of memory");
    }

    opterr=1;

    while ( (opt=getopt(argc, argv, "d:t:o:")) != EOF ) {
	switch (opt) {
	case 'd':   root = optarg;
		    break;
	case 't':   template = optarg;
		    break;
	case 'o':   output = optarg;
		    break;
	default:    fprintf(stderr, "usage: %s [-d dir] [-t tempplate] [-o html] [file]\n", pgm);
		    exit(1);
	}
    }

    tmplfile = open_template(template);

    argc -= optind;
    argv += optind;


    if ( argc > 0 ) {
	int added_text=0;

	source = alloca(strlen(argv[0]) + strlen(".text") + 1);
	strcpy(source,argv[0]);

	if ( !freopen(source, "r", stdin) ) {
	    strcat(source, ".text");
	    added_text = 1;
	    if ( !freopen(source, "r", stdin) )
		fail("can't open either %s or %s", argv[0], source);
	}

	if ( !output ) {
	    char *p, *q;
	    output = alloca(strlen(source) + strlen(".html") + 1);

	    if ( added_text ) {
		strcpy(output, argv[0]);
		strcat(output, ".html");
	    }
	    else {
		strcpy(output, source);

		if (( p = strchr(output, '/') ))
		    q = strrchr(p+1, '.');
		else
		    q = strrchr(output, '.');

		if ( q )
		    *q = 0;
		strcat(q, ".html");
	    }
	}
	if ( !freopen(output, "w", stdout) )
	    fail("can't write to %s", output);
    }


    if ( (doc = mkd_in(stdin, 0)) == 0 )
	fail("can't read %s", source ? source : "stdin");

    if ( !mkd_compile(doc, 0) )
	fail("couldn't compile input");

    if ( tmplfile )
	spin(tmplfile,doc,stdout);
    else
	mkd_generatehtml(doc, stdout);

    mkd_cleanup(doc);
    exit(0);
}
