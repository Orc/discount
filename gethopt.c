/*
 * gehopt;  options processing with both single-character and whole-word
 *           options both introduced with -
 */

#include <stdio.h>
#include <string.h>

#include "gethopt.h"


void
hoptset(ctx, argc, argv)
struct h_context *ctx;
int argc;
char **argv;
{
    memset(ctx, 0, sizeof *ctx);
    ctx->argc = argc;
    ctx->argv = argv;
    ctx->optind = 1;
}


char *
hoptarg(ctx)
struct h_context *ctx;
{
    return ctx->optarg;
}

int
hoptind(ctx)
struct h_context *ctx;
{
    return ctx->optind;
}

char
hoptopt(ctx)
struct h_context *ctx;
{
    return ctx->optopt;
}


int
hopterr(ctx,val)
struct h_context *ctx;
{
    int old = ctx->opterr;
    
    ctx->opterr = !!val;
    return old;
}


struct h_opt *
gethopt(ctx, opts, nropts)
struct h_context *ctx;
struct h_opt *opts;
int nropts;
{
    int i;
    int dashes;
    

    if ( (ctx == 0) || ctx->optend || (ctx->optind >= ctx->argc) )
	return 0;
    
    ctx->optarg = 0;
    ctx->optopt = 0;
    
    if ( ctx->optchar == 0) {
	/* check for leading -
	 */
	if ( ctx->argv[ctx->optind][0] != '-' ) {
	    /* out of arguments */
	    ctx->optend = 1;
	    return 0;
	}

	if ( ctx->argv[ctx->optind][1] == 0
	  || strcmp(ctx->argv[ctx->optind], "--") == 0 ) {
	    /* option list finishes with - or -- token
	     */
	    ctx->optend = 1;
	    ctx->optind++;
	    return 0;
	}

	dashes = 1;
	if ( ctx->argv[ctx->optind][dashes] == '-' ) {
	    /* support GNU-style long option double-dash prefix
	     * (if gethopt is passed an unknown option with a double-dash
	     *  prefix, it won't match a word and then the second dash
	     *  will be scanned as if it was a regular old single-character
	     *  option.)
	     */
	    dashes = 2;
	}
	
	for ( i=0; i < nropts; i++ ) {
	    if ( ! opts[i].optword ) 
		continue;

	    if (strcmp(opts[i].optword, dashes+(ctx->argv[ctx->optind]) ) == 0 ) {
		if ( opts[i].opthasarg ) {
		    if ( ctx->argc > ctx->optind ) {
			ctx->optarg = ctx->argv[ctx->optind+1];
			ctx->optind += 2;
		    }
		    else {
			/* word argument with required arg at end of
			 *command line
			 */
			if ( ctx->opterr )
			    fprintf(stderr,
				    "%s: option requires an argument -- %s\n",
				    ctx->argv[0], opts[i].optword);
			ctx->optind ++;
			return HOPTERR;
		    }
		}
		else {
		    ctx->optind ++;
		}
		return &opts[i];
	    }
	}
	ctx->optchar = 1;
    }

    ctx->optopt = ctx->argv[ctx->optind][ctx->optchar++];

    if ( !ctx->optopt ) {
	/* fell off the end of this argument */
	ctx->optind ++;
	ctx->optchar = 0;
	return gethopt(ctx, opts, nropts);
    }

    for ( i=0; i<nropts; i++ ) {
	if ( opts[i].optchar == ctx->optopt ) {
	    /* found a single-char option!
	     */
	    if ( opts[i].opthasarg ) {
		if ( ctx->argv[ctx->optind][ctx->optchar] ) {
		    /* argument immediately follows this options (-Oc)
		     */
		    ctx->optarg = &ctx->argv[ctx->optind][ctx->optchar];
		    ctx->optind ++;
		    ctx->optchar = 0;
		}
		else if ( ctx->optind < ctx->argc-1 ) {
		    /* argument is next arg (-O c)
		     */
		    ctx->optarg = &ctx->argv[ctx->optind+1][0];
		    ctx->optind += 2;
		    ctx->optchar = 0;
		}
		else {
		    /* end of arg string (-O); set optarg to null, return
		     * (should it opterr on me?)
		     */
		    ctx->optarg = 0;
		    ctx->optind ++;
		    ctx->optchar = 0;
		    if ( ctx->opterr )
			fprintf(stderr,
				"%s: option requires an argument -- %c\n",
				ctx->argv[0], opts[i].optchar);
		    return HOPTERR;
		}
	    }
	    else {
		if ( !ctx->argv[ctx->optind][ctx->optchar] ) {
		    ctx->optind ++;
		    ctx->optchar = 0;
		}
	    }
	    return &opts[i];
	}
    }
    if ( ctx->opterr )
	fprintf(stderr, "%s: illegal option -- %c\n", ctx->argv[0], ctx->optopt);
    return HOPTERR;
}


void
hoptusage(char *pgm, struct h_opt opts[], int nropts, char *arguments)
{
    int i;
    int optcount;
    
    fprintf(stderr, "usage: %s", pgm);

    /* print out the options that don't have flags first */
    
    for ( optcount=i=0; i < nropts; i++ ) {
	if ( opts[i].optchar && !opts[i].opthasarg) {
	    if (optcount == 0 )
		fputs(" [-", stderr);
	    fputc(opts[i].optchar, stderr);
	    optcount++;
	}
    }
    if ( optcount )
	fputc(']', stderr);

    /* print out the options WITH flags */
    for ( i = 0; i < nropts; i++ )
	if ( opts[i].optchar && opts[i].opthasarg)
	    fprintf(stderr, " [-%c %s]", opts[i].optchar, opts[i].opthasarg);

    /* print out the long options */
    for ( i = 0; i < nropts; i++ )
	if ( opts[i].optword ) {
	    fprintf(stderr, " [-%s", opts[i].optword);
	    if ( opts[i].opthasarg )
		fprintf(stderr, " %s", opts[i].opthasarg);
	    fputc(']', stderr);
	}

    /* print out the arguments string, if any */

    if ( arguments )
	fprintf(stderr, " %s", arguments);

    /* and we're done */
    fputc('\n', stderr);
}


#if DEBUG
struct h_opt opts[] = {
    { 0, "css",    0,  1, "css file" },
    { 1, "header", 0,  1, "header file" },
    { 2, 0,       'a', 0, "option a (no arg)" },
    { 3, 0,       'b', 1, "option B (with arg)" },
    { 4, "help",  '?', 0, "help message" },
} ;

#define NROPT (sizeof opts/sizeof opts[0])


int
main(argc, argv)
char **argv;
{
    struct h_opt *ret;
    struct h_context ctx;
    int i;


    hoptset(&ctx, argc, argv);
    hopterr(&ctx, 1);

    while (( ret = gethopt(&ctx, opts, NROPT) )) {

	if ( ret != HOPTERR ) {
	    if ( ret->optword )
		printf("%s", ret->optword);
	    else
		printf("%c", ret->optchar);

	    if ( ret->opthasarg ) {
		if ( hoptarg(&ctx) )
		    printf(" with argument \"%s\"", hoptarg(&ctx));
		else
		    printf(" with no argument?");
	    }
	    printf(" (%s)\n", ret->optdesc);
	}
    }

    argc -= hoptind(&ctx);
    argv += hoptind(&ctx);

    for ( i=0; i < argc; i++ )
	printf("%d: %s\n", i, argv[i]);
    return 0;
}

#endif /*DEBUG*/
