#ifndef _MKDIO_D
#define _MKDIO_D

#include <stdio.h>

/* line builder for markdown()
 */
void *mkd_in(FILE*,int);		/* assemble input from a file */
void *mkd_string(char*,int,int);	/* assemble input from a buffer */

/* compilation, debugging, cleanup
 */
int mkd_compile(void*, int);
int mkd_generatehtml(void*,FILE*);
int mkd_cleanup(void*);

/* markup functions
 */
int mkd_text(char *, int, FILE*, int);
int mkd_dump(void*, FILE*, int, char*);
int markdown(void*, FILE*, int);

/* header block access
 */
char* mkd_doc_title(void*);
char* mkd_doc_author(void*);
char* mkd_doc_date(void*);

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
