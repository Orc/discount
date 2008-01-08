/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "config.h"

#include "cstring.h"
#include "markdown.h"

/* block-level tags for passing html blocks through the blender
 */
static char *blocktags[] = { "ADDRESS", "BDO", "BLOCKQUOTE", "CENTER",
                      "DFN", "DIV", "H1", "H2", "H3", "H4",
		      "H5", "H6", "LISTING", "NOBR", "UL",
		      "P", "OL", "DL", "PLAINTEXT", "PRE",
		       "WBR", "XMP" };
#define SZTAGS	(sizeof blocktags / sizeof blocktags[0])

typedef int (*stfu)(const void*,const void*);

typedef ANCHOR(Paragraph) Document;



/* externals from generate.c */
void __mkd_generatehtml(Paragraph *, MMIOT *);
void __mkd_reparse(char *, int, int, MMIOT *);


/* case insensitive string sort (for qsort() and bsearch() of block tags)
 */
static int
casort(char **a, char **b)
{
    return strcasecmp(*a,*b);
}


/* case insensitive string sort for Footnote tags.
 */
int
__mkd_footsort(Footnote *a, Footnote *b)
{
    return strcasecmp(T(a->tag), T(b->tag));
}


static void
freeLine(Line *ptr)
{
    DELETE(ptr->text);
    free(ptr);
}


static void
freeLines(Line *p)
{
    if (p->next)
	 freeLines(p->next);
    freeLine(p);
}


static void
freeParagraph(Paragraph *p)
{
    if (p->next)
	freeParagraph(p->next);
    if (p->down)
	freeParagraph(p->down);
    if (p->text)
	freeLines(p->text);
    free(p);
}


static void
freefootnotes(MMIOT *f)
{
    int i;

    for (i=0; i < S(f->footnotes); i++) {
	DELETE(T(f->footnotes)[i].tag);
	DELETE(T(f->footnotes)[i].link);
	DELETE(T(f->footnotes)[i].title);
    }
    DELETE(f->footnotes);
}


static void
freeLineRange(Line *anchor, Line *stop)
{
    Line *r = anchor->next;

    if ( r != stop ) {
	while ( r && (r->next != stop) )
	    r = r->next;
	r->next = 0;
	freeLines(anchor->next);
    }
    anchor->next = 0;
}


/* see if a <tag> is one of the block tags that frames a html
 * paragraph.
 */
static char *
isblocktag(char *tag)
{
    char **r=bsearch(&tag, blocktags, SZTAGS, sizeof blocktags[0], (stfu)casort);

    return r ? (*r) : 0;
}


/* find the first blank character after position <i>
 */
static int
nextblank(Line *t, int i)
{
    while ( (i < S(t->text)) && !isspace(T(t->text)[i]) )
	++i;
    return i;
}


/* find the next nonblank character after position <i>
 */
static int
nextnonblank(Line *t, int i)
{
    while ( (i < S(t->text)) && isspace(T(t->text)[i]) )
	++i;
    return i;
}


/* find the first nonblank character on the Line.
 */
int
mkd_firstnonblank(Line *p)
{
    return nextnonblank(p,0);
}


static int
blankline(Line *p)
{
    return ! (p && (S(p->text) > p->dle) );
}


static Line *
skipempty(Line *p)
{
    while ( p && (p->dle == S(p->text)) )
	p = p->next;
    return p;
}


static char *
isopentag(Line *p)
{
    int i=0, len;
    char *key;

    if ( !p ) return 0;

    len = S(p->text);

    if ( len < 3 || T(p->text)[0] != '<' )
	return 0;

    for (i=1; (i < len) && isalnum(T(p->text)[i]); ++i)
	/* skip over tag (don't care about arguments or '>' yet) */ ;

    key = alloca(i);
    memcpy(key, T(p->text)+1, i-1);
    key[i-1] = 0;

    return isblocktag(key);
}


static int
isclosetag(Line *p, char *key, int siz)
{
    int eol;
    
    if ( !p ) return 0;

    eol = S(p->text)-1;

    if ( T(p->text)[eol] != '>' ) return 0;
    if ( strncasecmp(T(p->text)+(eol-siz), key, siz) != 0 ) return 0;
    if ( T(p->text)[eol-(siz+1)] != '/' ) return 0;
    if ( T(p->text)[eol-(siz+2)] != '<' ) return 0;
    return 1;
}


static Line *
htmlblock(Paragraph *p, char *key)
{
    Line *t, *ret;
    int keysize = strlen(key);

    for ( t = p->text; t ; t = t->next)
	if ( isclosetag(t, key, keysize) && blankline(t->next) ) {
	    ret = t->next;
	    t->next = 0;
	    return ret;
	}
    return t;
}


/* footnotes look like ^<whitespace>{0,3}[stuff]: <content>$
 */
static int
isfootnote(Line *t)
{
    int i;

    if ( ( (i = t->dle) > 3) || (T(t->text)[i] != '[') )
	return 0;

    for ( ; i < S(t->text) ; ++i ) {
	if ( T(t->text)[i] == ']' && T(t->text)[i+1] == ':' )
	    return 1;
    }
    return 0;
}


static int
islabel(char *line)
{
    return strchr("*-+", line[0]) && isspace(line[1]);
}

static int
isquote(Line *t)
{
    return ( T(t->text)[0] == '>' );
}


static int
dashchar(char c)
{
    return (c == '*') || (c == '-') || (c == '_');
}


static int
iscode(Line *t)
{
    return (t->dle >= 4);
}


static int
ishr(Line *t)
{
    int i, count=0;
    char dash = 0;
    char c;

    if ( iscode(t) ) return 0;

    for ( i = 0; i < S(t->text); i++) {
	c = T(t->text)[i];
	if ( (dash == 0) && dashchar(c) )
	    dash = c;

	if ( c == dash ) ++count;
	else if ( !isspace(c) )
	    return 0;
    }
    return (count >= 3);
}


static int
ishdr(Line *t, int *htyp)
{
    int i, j;


    /* first check for etx-style ###HEADER###
     */

    /* leading run of `#`'s ?
     */
    for ( i=0; T(t->text)[i] == '#'; ++i)
	;

    if ( i ) {
	i = nextnonblank(t, i);

	j = S(t->text)-1;

	while ( (j > i) && (T(t->text)[j] == '#') )
	    --j;
	
	while ( (j > 1) && isspace(T(t->text)[j]) )
	    --j;

	if ( i < j ) {
	    *htyp = ETX;
	    return 1;
	}
    }

    /* then check for setext-style HEADER
     *                             ======
     */

    if ( t->next ) {
	char *q = T(t->next->text);

	if ( (*q == '=') || (*q == '-') ) {
	    for (i=1; i < S(t->next->text); i++)
		if ( q[0] != q[i] )
		    return 0;
	    *htyp = SETEXT;
	    return 1;
	}
    }
    return 0;
}


#if DL_TAG_EXTENSION
static int
isdefinition(Line *t)
{
    return t && t->next && (S(t->text) > 2)
			&& (t->dle == 0)
			&& (t->next->dle >= 4)
			&& (T(t->text)[0] == '=')
			&& (T(t->text)[S(t->text)-1] == '=');
}
#endif


static int
islist(Line *t, int *trim)
{
    int i, j;
    char *q;
    
    if ( iscode(t) || blankline(t) || ishdr(t,&i) || ishr(t) )
	return 0;

#if DL_TAG_EXTENSION
    if ( isdefinition(t) ) {
	*trim = 4;
	return DL;
    }
#endif
    
    if ( islabel(T(t->text) + t->dle) ) {
	i = nextnonblank(t, t->dle+1);
	*trim = i;
	return UL;
    }

    if ( (j = nextblank(t,t->dle)) > t->dle ) {
	if ( T(t->text)[j-1] == '.' ) {
	    strtoul(T(t->text)+t->dle, &q, 10);
	    if ( q == T(t->text) + (j-1) ) {
		j = nextnonblank(t,j);
		*trim = j;
		return OL;
	    }
	}
    }
    return 0;
}


static Line *
headerblock(Paragraph *pp, int htyp)
{
    Line *ret = 0;
    Line *p = pp->text;
    int i, j;

    switch (htyp) {
    case SETEXT:
	    /* p->text is header, p->next->text is -'s or ='s
	     */
	    pp->hnumber = (T(p->next->text)[0] == '=') ? 1 : 2;
	    
	    ret = p->next->next;
	    freeLine(p->next);
	    p->next = 0;
	    break;

    case ETX:
	    /* p->text is ###header###, so we need to trim off
	     * the leading and trailing `#`'s
	     */

	    for (i=0; T(p->text)[i] == T(p->text)[0]; i++)
		;

	    pp->hnumber = i;
	    CLIP(p->text, 0, i);

	    for (j=S(p->text); j && (T(p->text)[j-1] == '#'); --j)
		;

	    CLIP(p->text,j, S(p->text)-j);
	    ret = p->next;
	    p->next = 0;
	    break;
    }
    return ret;
}


static int
fancy(Line *t)
{
    int z;

    return isquote(t) || iscode(t) || islist(t,&z) || ishdr(t,&z) || ishr(t);
}


static Line *
codeblock(Paragraph *p)
{
    Line *t, *r, *ret;

    for ( t = p->text; t; t = r ) {
	CLIP(t->text,0,4);
	t->dle = mkd_firstnonblank(t);

	r = t->next;
	if ( r && !iscode(r) ) {
	    ret = t->next;
	    t->next = 0;
	    return ret;
	}
    }
    return t;
}


static int
centered(Line *first, Line *last)
{

    if ( first&&last ) {
	int len = S(last->text);

	if ( (len > 2) && (strncmp(T(first->text), "->", 2) == 0)
		       && (strncmp(T(last->text)+len-2, "<-", 2) == 0) ) {
	    CLIP(first->text, 0, 2);
	    S(last->text) -= 2;
	    return CENTER;
	}
    }
    return 0;
}


static Line *
textblock(Paragraph *p)
{
    Line *t, *next;

    for ( t = p->text; t ; t = next )
	if ( ((next = t->next) == 0) || fancy(next) || blankline(next) ) {
	    p->align = centered(p->text, t);
	    t->next = 0;
	    return next;
	}
    return t;
}


static int
quoteprefix(Line *t)
{
    if ( T(t->text)[0] != '>' )
	return 0;

    return ( T(t->text)[1] == ' ' ) ? 2 : 1;
}


static Line *
quoteblock(Paragraph *p)
{
    Line *t, *q;
    int qp;

    for ( t = p->text; t ; t = q ) {
	if ( (qp = quoteprefix(t)) > 0 ) {
	    CLIP(t->text, 0, qp);
	    t->dle = mkd_firstnonblank(t);
	}

	if ( !(q = skipempty(t->next)) || ((q != t->next) && !isquote(q)) ) {
	    freeLineRange(t, q);
	    return q;
	}
    }
    return t;
}


static Paragraph *Pp(Document *, Line *, int);
static Paragraph *compile(Line *, int, MMIOT *);


/*
 * pull in a list block.  A list block starts with a list marker and
 * runs until the next list marker, the next non-indented paragraph,
 * or EOF.   You do not have to indent nonblank lines after the list
 * marker, but multiple paragraphs need to start with a 4-space indent.
 */
static Line *
listitem(Paragraph *p, int trim)
{
    Line *t, *q;

    for ( t = p->text; t ; t = q) {
	CLIP(t->text, 0, trim);
	t->dle = mkd_firstnonblank(t);

	if ( (q = skipempty(t->next)) == 0 ) {
	    freeLineRange(t,q);
	    return 0;
	}

	if ( islist(q, &trim) || ishr(q) || ((q != t->next) && (q->dle < 4)) ) {
	    q = t->next;
	    t->next = 0;
	    return q;
	}

	trim = (q->dle < 4) ? q->dle : 4;
    }
    return t;
}


static Line *
listblock(Paragraph *top, int trim, MMIOT *f)
{
    Document d = { 0, 0 };
    Paragraph *p;
    Line *q = top->text, *text;
    Line *label;
    int para = 0;

    while (( text = q )) {
	if ( top->typ == DL ) {
	    label = text;
	    text = text->next;
	    label->next = 0;
	}
	p = Pp(&d, text, LISTITEM);
	    
	text = listitem(p, trim);
	p->down = compile(p->text, 0, f);

	if ( top->typ == DL ) {
	    /* trim off leading > and trailing <
	     */
	    CLIP(label->text, 0, 1);
	    S(label->text)--;
	    p->text = label;
	}
	else
	    p->text = 0;

	if ( para ) p->down->align = PARA;

	if ( !(q = skipempty(text)) || (islist(q,&trim) != top->typ) )
	    break;

	para = (q != text);

	if ( para ) p->down->align = PARA;
    }
    top->text = 0;
    top->down = T(d);
    return text;
}


static int
tgood(char c)
{
    switch (c) {
    case '\'':
    case '"': return c;
    case '(': return ')';
    }
    return 0;
}


/*
 * add a new (image or link) footnote to the footnote table
 */
static Line*
addfootnote(Line *p, MMIOT* f)
{
    int j, i;
    int c;
    Line *p2 = p->next;

    Footnote *foot = &EXPAND(f->footnotes);
    
    CREATE(foot->tag);
    CREATE(foot->link);
    CREATE(foot->title);
    foot->height = foot->width = 0;

    for (j=i=p->dle; T(p->text)[j] != ']'; j++)
	EXPAND(foot->tag) = T(p->text)[j];

    EXPAND(foot->tag) = T(p->text)[j++];
    EXPAND(foot->tag) = 0;
    j = nextnonblank(p, j+1);

    while ( (j < S(p->text)) && !isspace(T(p->text)[j]) )
	EXPAND(foot->link) = T(p->text)[j++];
    EXPAND(foot->link) = 0;
    j = nextnonblank(p,j);

    if ( T(p->text)[j] == '=' ) {
	sscanf(T(p->text)+j, "=%dx%d", &foot->width, &foot->height);
	while ( (j < S(p->text)) && !isspace(T(p->text)[j]) )
	    ++j;
	j = nextnonblank(p,j);
    }


    if ( (j >= S(p->text)) && p2 && p2->dle && tgood(T(p2->text)[p2->dle]) ) {
	j = p2->dle;
	p = p2;
    }

    if ( (c = tgood(T(p->text)[j])) ) {
	while ( (j < S(p->text)-1) && (T(p->text)[++j] != c) )
	    EXPAND(foot->title) = T(p->text)[j];
	EXPAND(foot->title) = 0;
    }

    return p->next;
}


/*
 * allocate a paragraph header, link it to the
 * tail of the current document
 */
static Paragraph *
Pp(Document *d, Line *ptr, int typ)
{
    Paragraph *ret = calloc(sizeof *ret, 1);

    ret->text = ptr;
    ret->typ = typ;

    return ATTACH(*d, ret);
}



static Line*
consume(Line *ptr, int *eaten)
{
    Line *next;

    for ( ; ptr && blankline(ptr); ptr = next, *eaten = 1 ) {
	next = ptr->next;
	freeLine(ptr);
    }
    return ptr;
}


/*
 * break a collection of markdown input into
 * blocks of lists, code, html, and text to
 * be marked up.
 */
static Paragraph *
compile(Line *ptr, int toplevel, MMIOT *f)
{
    Document d = { 0, 0 };
    Paragraph *p = 0;
    char *key;
    int para = toplevel;
    int hdr_type, list_type, indent;

    ptr = consume(ptr, &para);

    while ( ptr ) {
	if ( toplevel && (key = isopentag(ptr)) ) {
	    p = Pp(&d, ptr, HTML);
	    ptr = htmlblock(p, key);
	}
	else if ( iscode(ptr) ) {
	    p = Pp(&d, ptr, CODE);
	    ptr = codeblock(p);
	}
	else if ( ishr(ptr) ) {
	    p = Pp(&d, 0, HR);
	    ptr = ptr->next;
	}
	else if ( ishdr(ptr, &hdr_type) ) {
	    p = Pp(&d, ptr, HDR);
	    ptr = headerblock(p, hdr_type);
	}
	else if (( list_type = islist(ptr, &indent) )) {
	    p = Pp(&d, ptr, list_type);
	    ptr = listblock(p, indent, f);
	}
	else if ( isquote(ptr) ) {
	    p = Pp(&d, ptr, QUOTE);
	    ptr = quoteblock(p);
	    p->down = compile(p->text, 0, f);
	    p->text = 0;
	}
	else if ( toplevel && (isfootnote(ptr)) ) {
	    ptr = consume(addfootnote(ptr, f), &para);
	    continue;
	}
	else {
	    p = Pp(&d, ptr, MARKUP);
	    ptr = textblock(p);
	}

	if ( para && !p->align )
	    p->align = PARA;

	para = toplevel;
	ptr = consume(ptr, &para);

	if ( para && !p->align )
	    p->align = PARA;

    }
    return T(d);
}


static void
initmarkdown()
{
    static int first = 1;

    if ( first-- > 0 ) {
	first = 0;
	srandom((unsigned int)time(0));
	qsort(blocktags, SZTAGS, sizeof blocktags[0], (stfu)casort);
    }
}


/* 
 */
int
mkd_text(char *bfr, int size, FILE *output, int flags)
{
    MMIOT f;

    f.out = output;
    f.flags = flags;
    
    initmarkdown();
    __mkd_reparse(bfr, size, 0, &f);
    return 0;
}


/*
 * the guts of the markdown() function, ripped out so I can do
 * debugging.
 */

/*
 * prepare and compile `text`, returning a Paragraph tree.
 */
Paragraph *
__mkd_compile(Line *text, FILE *out, int flags, MMIOT *ctx)
{
    Paragraph *paragraph;

    bzero(ctx, sizeof *ctx);
    ctx->out = out;
    ctx->flags = flags;
    CREATE(ctx->in);
    CREATE(ctx->footnotes);

    initmarkdown();

    paragraph = compile(text, 1, ctx);
    qsort(T(ctx->footnotes), S(ctx->footnotes), sizeof T(ctx->footnotes)[0],
						       (stfu)__mkd_footsort);

    return paragraph;
}


/* clean up everything allocated in __mkd_compile()
 */
void
__mkd_cleanup(Paragraph *tree, MMIOT *ctx)
{
    freefootnotes(ctx);
    if ( tree ) freeParagraph(tree);
    DELETE(ctx->in);
}


/* convert some markdown text to html
 */
int
markdown(Line *text, FILE *out, int flags)
{
    MMIOT f;
    Paragraph *pp;

    if ( !text ) return 0;

    if (( pp = __mkd_compile(text, out, flags, &f) )) {
	__mkd_generatehtml(pp, &f);
	__mkd_cleanup(pp, &f);
	return 0;
    }
    return -1;
}
