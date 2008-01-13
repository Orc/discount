#ifndef _MKDIO_D
#define _MKDIO_D

#include <stdio.h>

typedef void *MMIOT;

/* line builder for markdown()
 */
MMIOT mkd_in(FILE*,int);		/* assemble input from a file */
MMIOT mkd_string(char*,int,int);	/* assemble input from a buffer */

/* compilation, debugging, cleanup
 */
int mkd_compile(MMIOT, int);
int mkd_generatehtml(MMIOT,FILE*);
int mkd_cleanup(MMIOT);

/* markup functions
 */
int mkd_text(char *, int, FILE*, int);
int mkd_dump(MMIOT, FILE*, int, char*);
int markdown(MMIOT, FILE*, int);

/* header block access
 */
char* mkd_doc_title(MMIOT);
char* mkd_doc_author(MMIOT);
char* mkd_doc_date(MMIOT);

/* special flags for markdown() and mkd_text()
 */
#define MKD_NOLINKS	0x0001	/* don't do link processing, block <a> tags  */
#define MKD_NOIMAGE	0x0002	/* don't do image processing, block <img> */
#define MKD_NOPANTS	0x0004	/* don't run smartypants() */
#define MKD_QUOT	0x0010	/* expand " to &quot; */

/* special flags for mkd_in() and mkd_string()
 */
#define MKD_NOHEADER	0x0100	/* don't process header blocks */

#endif/*_MKDIO_D*/
