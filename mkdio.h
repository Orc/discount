#ifndef _MKDIO_D
#define _MKDIO_D

#include <stdio.h>

/* line builder for markdown()
 */
void *mkd_in(FILE*);			/* assemble input from a file */
void *mkd_string(char*,int);		/* assemble input from a buffer */

/* compilation, debugging, cleanup
 */
int mkd_compile(void*, FILE*, int, void*);
int mkd_dump(void*, FILE*, int, char*);
int mkd_cleanup(void*, void*);

/* markup functions
 */
int mkd_text(char *, int, FILE*, int);
int markdown(void*, FILE*, int);

/* special flags for markdown() and mkd_text()
 */
#define MKD_NOLINKS	0x01	/* don't do link processing, block <a> tags  */
#define MKD_NOIMAGE	0x02	/* don't do image processing, block <img> */
#define MKD_NOPANTS	0x04	/* don't run smartypants() */

#endif/*_MKDIO_D*/
