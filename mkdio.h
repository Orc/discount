#ifndef _MKDIO_D
#define _MKDIO_D

#include <stdio.h>

void *mkd_open();			/* open a mkdio input assembler */
int   mkd_write(void*, char*, int);	/* write text into the assembler */
void *mkd_close(void*);			/* get the assembled input */

void *mkd_in(FILE*);			/* get assembled input from a file */

void markdown(void*, FILE*, int);	/* mark it on down */

#define MKD_NOLINKS	0x01
#define MKD_NOIMAGE	0x02

#endif/*_MKDIO_D*/
