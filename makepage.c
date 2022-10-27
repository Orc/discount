/*
 * makepage: Use mkd_xhtmlpage() to convert markdown input to a
 *           fully-formed xhtml page.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mkdio.h>
#include "config.h"
#include "pgm_options.h"
#include "gethopt.h"

#ifndef HAVE_BASENAME
char*
basename(char *p)
{
    char *ret = strrchr(p, '/');

    return ret ? (1+ret) : p;
}
#endif

char *pgm = "makepage";

struct h_opt opts[] = {
    { 0, "version", 'V', 0,           "show version info" },
    { 0, 0,         'F', "bitmap",    "set/show hex flags" },
    { 0, "flags",   'f', "{+-}flags", "set/show named flags" },
} ;
#define NROPTS (sizeof opts / sizeof opts[0])

int
main(int argc, char **argv)
{
    MMIOT *doc;
    char *q;
    int version = 0;
    int ret, i;
    DWORD bits;
    mkd_flag_t *flags = mkd_flags();
    struct h_opt *opt;
    struct h_context blob;

    if ( !flags ) 
	perror("mkd_flags");
    
    if ( (q = getenv("MARKDOWN_FLAGS")) ) {
	bits = strtol(q, 0, 0);

	for ( i=0; i < 8*sizeof(bits); i++)
	    if ( bits & (1<<i) )
		mkd_set_flag_num(flags, i);
    }

    hoptset(&blob, argc, argv);
    hopterr(&blob, 1);

    while ( opt = gethopt(&blob, opts, NROPTS) ) {
	if ( opt == HOPTERR ) {
	    hoptusage(pgm, opts, NROPTS, "[file]");
	    exit(1);
	}
	switch ( opt->optchar ) {
	case 'V':   version++;
		    break;
	case 'F':   if ( strcmp(hoptarg(&blob), "?") == 0 ) {
			show_flags(0,0,0);
			exit(0);
		    }
		    else {
			bits = strtol(hoptarg(&blob), 0, 0);

			for (i=0; i < 8*sizeof(bits); i++)
			    if ( bits & (1<<i) )
				mkd_set_flag_num(flags, i);
		    }
		    break;
	case 'f':   if ( strcmp(hoptarg(&blob), "?") == 0 ) {
			show_flags(1,version,0);
			exit(0);
		    }
		    else if ( q = mkd_set_flag_string(flags, hoptarg(&blob)) )
			fprintf(stderr, "unknown option <%s>\n", q);
		    break;
	}
    }
    
    argc -= hoptind(&blob);
    argv += hoptind(&blob);
    
    if ( version ) {
	printf("%s: discount %s", pgm, markdown_version);
	if ( version > 1 )
	    mkd_flags_are(stdout, flags, 0);
	putchar('\n');
	exit(0);
    }    
    
    if ( (argc > 0) && !freopen(argv[0], "r", stdin) ) {
	perror(argv[0]);
	exit(1);
    }

    if ( (doc = mkd_in(stdin, flags)) == 0 ) {
	perror( (argc > 1) ? argv[1] : "stdin" );
	exit(1);
    }

    ret = mkd_xhtmlpage(doc, flags, stdout);

    mkd_cleanup(doc);

    mkd_free_flags(flags);

    return (ret == EOF);
}
