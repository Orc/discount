#ifndef _MKDIO_D
#define _MKDIO_D

#include <stdio.h>

void *mkd_open();			/* open a mkdio input assembler */
int   mkd_write(void*, char*, int);	/* write text into the assembler */
void *mkd_close(void*);			/* get the assembled input */

void *mkd_in(FILE*);			/* assemble input from a file */
void *mkd_string(char*,int);		/* assemble input from a buffer */

void markdown(void*, FILE*, int);	/* mark it on down */

void mkd_push(char *, int);
void mkd_text(FILE *out);

#define MKD_NOLINKS	0x01
#define MKD_NOIMAGE	0x02

#endif/*_MKDIO_D*/
