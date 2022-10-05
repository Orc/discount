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

/*
 * discount2 shim layer
 */

/* 2.2.6 flags */
#define MKD2_NOLINKS		0x00000001
#define MKD2_NOIMAGE		0x00000002
#define MKD2_NOPANTS		0x00000004
#define MKD2_NOHTML		0x00000008
#define MKD2_STRICT		0x00000010
#define MKD2_TAGTEXT		0x00000020
#define MKD2_NO_EXT		0x00000040
#define MKD2_CDATA		0x00000080
#define MKD2_NOSUPERSCRIPT	0x00000100
#define MKD2_STRICT2   		0x00000200
#define MKD2_NOTABLES		0x00000400
#define MKD2_NOSTRIKETHROUGH	0x00000800
#define MKD2_TOC		0x00001000
#define MKD2_1_COMPAT		0x00002000
#define MKD2_AUTOLINK		0x00004000
#define MKD2_SAFELINK		0x00008000
#define MKD2_NOHEADER		0x00010000
#define MKD2_TABSTOP		0x00020000
#define MKD2_NODIVQUOTE		0x00040000
#define MKD2_NOALPHALIST	0x00080000
#define MKD2_NODLIST		0x00100000
#define MKD2_EXTRA_FOOTNOTE	0x00200000
#define MKD2_NOSTYLE		0x00400000
#define MKD2_NODLDISCOUNT	0x00800000
#define	MKD2_DLEXTRA		0x01000000
#define MKD2_FENCEDCODE		0x02000000
#define MKD2_IDANCHOR		0x04000000
#define MKD2_GITHUBTAGS		0x08000000
#define MKD2_URLENCODEDANCHOR	0x10000000
#define MKD2_LATEX		0x40000000
#define MKD2_EXPLICITLIST	0x80000000


/*
 * convert a discount2 bitmap flag into a discount3 flag blob
 */
static void
convert_v2flags(DWORD bitmask, mkd_flag_t *blob)
{
    int i;
    DWORD bit;

    mkd_init_flags(blob);
    set_mkd_flag(blob, MKD_DLDISCOUNT);	/* default for discount dls changed from on to off in v3 */


    
    for (i=0; i < sizeof(DWORD)*8; i++) {

	bit = 1L << i;

	switch ( bitmask & bit ) {
	case MKD2_STRICT:	set_mkd_flag(blob,MKD_STRICT);
				break;
	case MKD2_NOLINKS:	set_mkd_flag(blob,MKD_NOLINKS);
				break;
	case MKD2_NOIMAGE:	set_mkd_flag(blob,MKD_NOIMAGE);
				break;
	case MKD2_NOPANTS:	set_mkd_flag(blob,MKD_NOPANTS);
				break;
	case MKD2_NOHTML:	set_mkd_flag(blob,MKD_NOHTML);
				break;
	case MKD2_TAGTEXT:	set_mkd_flag(blob,MKD_TAGTEXT);
				break;
	case MKD2_NO_EXT:	set_mkd_flag(blob,MKD_NO_EXT);
				break;
	case MKD2_CDATA:	set_mkd_flag(blob,MKD_CDATA);
				break;
	case MKD2_NOSUPERSCRIPT:set_mkd_flag(blob,MKD_NOSUPERSCRIPT);
				break;
	case MKD2_STRICT2:	set_mkd_flag(blob,MKD_STRICT);
				break;
	case MKD2_NOTABLES:	set_mkd_flag(blob,MKD_NOTABLES);
				break;
	case MKD2_NOSTRIKETHROUGH: set_mkd_flag(blob,MKD_NOSTRIKETHROUGH);
				break;
	case MKD2_TOC:		set_mkd_flag(blob,MKD_TOC);
				break;
	case MKD2_1_COMPAT:	set_mkd_flag(blob,MKD_1_COMPAT);
				break;
	case MKD2_AUTOLINK:	set_mkd_flag(blob,MKD_AUTOLINK);
				break;
	case MKD2_SAFELINK:	set_mkd_flag(blob,MKD_SAFELINK);
				break;
	case MKD2_NOHEADER:	set_mkd_flag(blob,MKD_NOHEADER);
				break;
	case MKD2_TABSTOP:	set_mkd_flag(blob,MKD_TABSTOP);
				break;
	case MKD2_NODIVQUOTE:	set_mkd_flag(blob,MKD_NODIVQUOTE);
				break;
	case MKD2_NOALPHALIST:	set_mkd_flag(blob,MKD_NOALPHALIST);
				break;
	case MKD2_EXTRA_FOOTNOTE: set_mkd_flag(blob,MKD_EXTRA_FOOTNOTE);
				break;
	case MKD2_NOSTYLE:	set_mkd_flag(blob,MKD_NOSTYLE);
				break;
	case MKD2_NODLDISCOUNT:	clear_mkd_flag(blob,MKD_DLDISCOUNT);
				break;
	case MKD2_DLEXTRA:	set_mkd_flag(blob,MKD_DLEXTRA);
				break;
	case MKD2_FENCEDCODE:	set_mkd_flag(blob,MKD_FENCEDCODE);
				break;
	case MKD2_IDANCHOR:	set_mkd_flag(blob,MKD_IDANCHOR);
				break;
	case MKD2_GITHUBTAGS:	set_mkd_flag(blob,MKD_GITHUBTAGS);
				break;
	case MKD2_URLENCODEDANCHOR: set_mkd_flag(blob,MKD_URLENCODEDANCHOR);
				break;
	case MKD2_LATEX:	set_mkd_flag(blob,MKD_LATEX);
				break;
	case MKD2_EXPLICITLIST:	set_mkd_flag(blob,MKD_EXPLICITLIST);
				break;
	}
    }
}

#undef mkd_in
MMIOT *mkd_in(FILE *input, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return (MMIOT*)mkd3_in(input, &v3flags);
}


#if 0
char *mkd3_set_flag_string(mkd_flag_t*, char*);	/* set named flags */
char *mkd_set_flag_string(mkd2_flag_t, char*);	/* set named flags */
#endif


#undef mkd_string
MMIOT *
mkd_string(const char *text, int length, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return (MMIOT*)mkd3_string(text,length, &v3flags);
}


#undef gfm_in
MMIOT *
gfm_in(FILE *input, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return (MMIOT*)gfm3_in(input, &v3flags);
}


#undef gfm_string
MMIOT *
gfm_string(const char *text, int length, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return (MMIOT*)gfm3_string(text, length, &v3flags);
}


#undef mkd_compile
int
mkd_compile(MMIOT *document, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return mkd3_compile((Document*)document, &v3flags);
}


#undef mkd_dump
int
mkd_dump(MMIOT *document, FILE *output, DWORD v2flags, char *word)
{
    extern int mkd3_dump(MMIOT*, FILE*, mkd_flag_t*, char*);
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return mkd3_dump(document, output, &v3flags, word);
}


#undef markdown
int
markdown(MMIOT *document, FILE *output, DWORD v2flags)
{
    extern int markdown3(MMIOT*, FILE*, mkd_flag_t*);
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return markdown3(document, output, &v3flags);
}


#undef mkd_line
int
mkd_line(char *text, int size, char **ret, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return mkd3_line(text, size, ret, &v3flags);
}


#undef mkd_xhtmlpage
int
mkd_xhtmlpage(MMIOT *document, DWORD v2flags,FILE *output)
{
    extern int mkd3_xhtmlpage(MMIOT*, mkd_flag_t*, FILE*);
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return mkd3_xhtmlpage(document, &v3flags, output);
}


#undef mkd_generateline
int
mkd_generateline(char *text, int size, FILE *output, DWORD v2flags)
{
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    return mkd3_generateline(text, size, output, &v3flags);
}


#undef mkd_flags_are
void
mkd_flags_are(FILE *output, DWORD v2flags, int htmlplease)
{
    extern void mkd3_flags_are(FILE *f, mkd_flag_t* flags, int htmlplease);
    mkd_flag_t v3flags;

    convert_v2flags(v2flags, &v3flags);

    mkd3_flags_are(output, &v3flags, htmlplease);
}
