/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007-2011 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "markdown.h"
#include "amalloc.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

void
mkd_set_flag_num(mkd_flag_t *p, unsigned long bit)
{
    if ( p && (bit < MKD_NR_FLAGS) )
	set_mkd_flag(p, bit);
}


void
mkd_clr_flag_num(mkd_flag_t *p, unsigned long bit)
{
    if ( p && (bit < MKD_NR_FLAGS) )
	clear_mkd_flag(p, bit);
}


void
mkd_set_flag_bitmap(mkd_flag_t *p, long bits)
{
    int i;
    
    if ( p == 0 )
	return;

    for (i=0; i < 8*sizeof(long) && i < MKD_NR_FLAGS; i++)
	if ( bits & (1<<i) )
	    set_mkd_flag(p, i);
}
