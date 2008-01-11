/*
 * docheader -- get values from the document header
 *
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "cstring.h"
#include "markdown.h"


char *
mkd_doc_title(Document *doc)
{
    if ( doc && doc->headers )
	return T(doc->headers->text);
    return 0;
}


char *
mkd_doc_author(Document *doc)
{
    if ( doc && doc->headers && doc->headers->next )
	return T(doc->headers->next->text);
    return 0;
}


char *
mkd_doc_date(Document *doc)
{
    if ( doc && doc->headers && doc->headers->next && doc->headers->next->next )
	return T(doc->headers->next->next->text);
    return 0;
}
