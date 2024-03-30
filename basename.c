/*
 * mkdio -- markdown front end input functions
 *
 * Copyright (C) 2007 Jessica L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "mkdio.h"
#include "cstring.h"
#include "amalloc.h"

static char *
e_basename(const char *string, const int size, void *context)
{
    char *ret;
    char *base = (char*)context;

    if ( base && string && (ret=malloc(strlen(base)+size+2)) ) {
	strcpy(ret, base);
	strncat(ret, string, size);
	return ret;
    }
    return 0;
}

static void
basename_free(char *p, int len, void *ctx)
{
    if ( p ) free(p);
}

void
mkd_basename(MMIOT *document, char *base)
{
    if ( document && base )
	mkd_e_url(document, e_basename, (mkd_free_t)basename_free, base);
}
