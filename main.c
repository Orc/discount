/*
 * markdown: convert a single markdown document into html
 */
/*
 * Copyright (C) 2007 Jessica L Parsons.
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
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#include "config.h"
#include "amalloc.h"
#include "pgm_options.h"
#include "tags.h"
#include "gethopt.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifndef HAVE_BASENAME
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


void
callback_free(char *input, int size, void *ctx)
{
    if (input)
	free(input);
}


char *
anchor_format(char *input, void *ctx)
{
    int i, j, size;
    char* ret;

    if ( !input )
	return NULL;

     size = strlen(input);

     ret = malloc(1+size);

     if ( !ret )
	 return NULL;


    while ( size && isspace(input[size-1]) )
	--size;

    for ( j=i=0; i < size; i++ ) {
	if (isalnum(input[i]) || strchr("-_+", input[i]) )
	    ret[j++] = input[i];
	else if ( input[i] == ' ' )
	    ret[j++] = '-';
    }
    ret[j++] = 0;

    return ret;
}

char *external_formatter = 0;


#define RECEIVER 0
#define SENDER 1

char *
external_codefmt(char *src, int len, char *lang)
{
    int child_status;
    int size, bufsize, curr;
    char *res;

    pid_t child;

    int tochild[2], toparent[2];

    if ( pipe(tochild) != 0 || pipe(toparent) != 0 ) {
	perror("external_codefmt (pipe)");
	res = malloc(len+1);
	strcpy(res, src);
	return res;
    }

    if ( (child = fork()) > 0 ) {

	close(tochild[RECEIVER]);
	close(toparent[SENDER]);

	bufsize = 1000;
	res = malloc(1+bufsize);
	curr = 0;

	/* parent */
	write(tochild[SENDER], src, len);
	close(tochild[SENDER]);

	while ( (size = read(toparent[RECEIVER], res+curr, 1000)) > 0 ) {
	    curr += size;
	    res = realloc(res, bufsize += 1000);
	}
	res[curr] = 0;
	waitpid(child, &child_status, 0);

	close(toparent[RECEIVER]);

	if ( WIFEXITED(child_status) )
	    return res;
	else
	    free(res);	/* something failed; just return the original string */
    }
    else if ( child == 0 ) {
	close(tochild[SENDER]);
	close(toparent[RECEIVER]);
	close(1); dup2(toparent[SENDER], 1);
	close(0); dup2(tochild[RECEIVER], 0);
	system(external_formatter);
	close(0); close(tochild[RECEIVER]);
	close(1); close(toparent[SENDER]);
	exit(0);
    }
    res = malloc(len+1);
    strcpy(res, src);
    return res;
}


struct h_opt opts[] = {
    { 0, "html5",  '5', 0,           "recognise html5 block elements" },
    { 0, "base",   'b', "url-base",  "URL prefix" },
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
    { 0, 0,        'o', "file",      "write output to file" },
    { 0, "squash", 'x', 0,           "squash toc labels to be more like github" },
    { 0, "codefmt",'X', "command",   "use an external code formatter" },
    { 0, "help",   '?', 0,           "print a detailed usage message" },
};
#define NROPTS (sizeof opts/sizeof opts[0])


int
main(int argc, char **argv)
{
    int rc;
    int debug = 0;
    int toc = 0;
    int content = 1;
    int version = 0;
    int with_html5 = 0;
    int styles = 0;
    int use_mkd_line = 0;
    int use_e_codefmt = 0;
    int github_flavoured = 0;
    int squash = 0;
    char *extra_footnote_prefix = 0;
    char *urlflags = 0;
    char *text = 0;
    char *ofile = 0;
    char *urlbase = 0;
    char *q;
    MMIOT *doc;
    struct h_context blob;
    struct h_opt *opt;
    mkd_flag_t *flags = mkd_flags();

    if ( !flags )
	perror("new_flags");

    hoptset(&blob, argc, argv);
    hopterr(&blob, 1);

    pgm = basename(argv[0]);

    if ( q = getenv("MARKDOWN_FLAGS") )
	mkd_set_flag_bitmap(flags, strtol(q,0,0));

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
	case 'd':   debug++;
		    break;
	case 'V':   version++;
		    break;
	case 'E':   urlflags = hoptarg(&blob);
		    break;
	case 'f':   q = hoptarg(&blob);
		    if ( strcmp(q, "?") == 0 ) {
			show_flags(1, version, 0);
			exit(0);
		    }
		    else if ( strcmp(q, "??") == 0 ) {
			show_flags(1, version, flags);
			exit(0);
		    }
		    else if ( q=mkd_set_flag_string(flags, hoptarg(&blob)) )
			complain("unknown option <%s>", q);
		    break;
	case 'F':   q = hoptarg(&blob);
		    if ( strcmp(q, "?") == 0 ) {
			show_flags(0, 0, 0);
			exit(0);
		    }
		    else if ( strcmp(q, "??") == 0 ) {
			show_flags(0, version, flags);
			exit(0);
		    }
		    else
			mkd_set_flag_bitmap(flags,strtol(q, 0, 0));
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
	case 'T':   mkd_set_flag_num(flags, MKD_TOC);
		    toc = 1;
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
	case 'x':   squash = 1;
		    break;
	case 'X':   use_e_codefmt = 1;
		    mkd_set_flag_num(flags, MKD_FENCEDCODE);
		    external_formatter = hoptarg(&blob);
		    fprintf(stderr, "selected external formatter (%s)\n", external_formatter);
		    break;
	case '?':   hoptdescribe(pgm, opts, NROPTS, "[file]", 1);
		    return 0;
	}
    }


    if ( version ) {
	printf("%s: discount %s%s", pgm, markdown_version,
				  with_html5 ? " +html5":"");
	if ( version == 2 )
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

	    doc = github_flavoured ? gfm_in(stdin,flags)
				   : mkd_in(stdin,flags);
	    if ( !doc ) {
		perror(argc ? argv[0] : "stdin");
		exit(1);
	    }
	}
	if ( urlbase )
	    mkd_basename(doc, urlbase);

	if ( urlflags )
	    mkd_e_flags(doc, e_flags, NULL, urlflags);

	if ( squash )
	    mkd_e_anchor(doc, (mkd_callback_t) anchor_format, callback_free, 0);

	if ( use_e_codefmt )
	    mkd_e_code_format(doc, (mkd_callback_t)external_codefmt, callback_free, 0);


	if ( extra_footnote_prefix )
	    mkd_ref_prefix(doc, extra_footnote_prefix);

	if ( debug ) {
	    rc = mkd_dump(doc, stdout, flags, argc ? basename(argv[0]) : "stdin");
	}
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
    mkd_free_flags(flags);
    adump();
    exit( (rc == 0) ? 0 : errno );
}
