/*
 * mkdio -- markdown front end input functions
 *
 * Copyright (C) 2007 David L Parsons.
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
    char *base = ((char **)context)[0];
    
    if ( base && string && (*string == '/') && (ret=malloc(strlen(base)+size+2)) ) {
	strcpy(ret, base);
	strncat(ret, string, size);
	return ret;
    }
    return 0;
}

static char *
e_anchorid(const char *string, const int size, void *context)
{
    char *anchorid = ((char **)context)[1];
    return strdup(anchorid);
}

static void
e_free(char *string, void *context)
{
    if ( string ) free(string);
}

/* basename uses index 0, anchorid 1 */
static char *data[2] = {0};

void
mkd_basename(MMIOT *document, char *base)
{
    data[0] = base;
    mkd_e_url(document, e_basename);
    mkd_e_data(document, data);
    mkd_e_free(document, e_free);
}

void
mkd_anchorid(MMIOT *document, char *anchorid)
{
    data[1] = anchorid;
    mkd_e_anchorid(document, e_anchorid);
    mkd_e_data(document, data);
    mkd_e_free(document, e_free);
}
