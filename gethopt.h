/*
 * gethopt;  options processing with both single-character and whole-work
 *           options both introduced with -
 */

#ifndef __GETHOPT_D
#define __GETHOPT_D

#include <stdio.h>
#include <string.h>


struct h_opt {
    int  option;
    char *optword;
    char optchar;
    char *opthasarg;
    char *optdesc;
} ;

#define HOPTERR	((struct h_opt*)-1)

struct h_context {
    char **argv;
    int    argc;
    int    optchar;
    int    optind;
    char  *optarg;
    char   optopt;
    int    opterr:1;
    int    optend:1;
} ;

extern char *hoptarg(struct h_context *);
extern int   hoptind(struct h_context *);
extern char  hoptopt(struct h_context *);
extern void  hoptset(struct h_context *, int, char **);
extern int   hopterr(struct h_context *, int);
extern struct h_opt *gethopt(struct h_context *, struct h_opt*, int);

extern void hoptusage(char *, struct h_opt*, int, char *);

#endif/*__GETHOPT_D*/
