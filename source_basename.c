/*
 * mkdio -- markdown front end input functions
 *
 * Copyright (C) 2014 David L Parsons.
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
sl_handler(const char *unused, const int line, void *context)
{
    char *base = (char*)context;

    if ( base && line > 0 ) {
	char *ret;
	int sz = strlen(base) + 20;

	ret = malloc(sz);
	if (ret && snprintf(ret, sz, "%s%d", base, line) > 0)
	    return ret;
    }
    return 0;
}

static void
sl_free(char *string, void *context)
{
    free(string);
}

void
mkd_source_basename(MMIOT *document, char *base)
{
    mkd_sl_handler(document, sl_handler);
    mkd_sl_free(document, sl_free);
    mkd_sl_data(document, base);
}
