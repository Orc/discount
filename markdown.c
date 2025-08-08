/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007 Jessica L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "cstring.h"
#include "markdown.h"
#include "amalloc.h"
#include "tags.h"

typedef int (*stfu)(const void*,const void*);

static Paragraph *Pp(ParagraphRoot *, Line *, int);
static Paragraph *compile(Line *, int, MMIOT *);

/* get i-th Cstring T member
 */
#define ithTMbr(s, i)		T(s)[i]


/* case insensitive string sort for Footnote tags.
 */
int
__mkd_footsort(Footnote *a, Footnote *b)
{
    int i;
    char ac, bc;

    if ( S(a->tag) != S(b->tag) )
	return S(a->tag) - S(b->tag);

    for ( i=0; i < S(a->tag); i++) {
	ac = tolower(ithTMbr(a->tag, i));
	bc = tolower(ithTMbr(b->tag, i));

	if ( isspace(ac) && isspace(bc) )
	    continue;
	if ( ac != bc )
	    return ac - bc;
    }
    return 0;
}


/* return end of digits string given an offset
 */
static int
endOfDigits(char *t, int start)
{
    int end;
    for ( end = start; isdigit(t[end]); ++end )
	;
    return end;
}

/* find the first blank character after position <i>
 */
static int
nextblank(Line *t, int i)
{
    while ( (i < S(t->text)) && !isspace(ithTMbr(t->text, i)) )
	++i;
    return i;
}


/* find the next nonblank character after position <i>
 */
static int
nextnonblank(Line *t, int i)
{
    while ( (i < S(t->text)) && isspace(ithTMbr(t->text, i)) )
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


static inline int
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


void
___mkd_tidy(Cstring *t)
{
    while ( S(*t) && isspace(ithTMbr(*t, S(*t)-1)) )
	--S(*t);
}


static struct kw comment = { "!--", 3, 0 };

static struct kw *
isopentag(Line *p)
{
    int i=0, len;
    char *line;

    if ( !p ) return 0;

    line = T(p->text);
    len = S(p->text);

    if ( len < 3 || line[0] != '<' )
	return 0;

    if ( line[1] == '!' && line[2] == '-' && line[3] == '-' )
	/* comments need special case handling, because
	 * the !-- doesn't need to end in a whitespace
	 */
	return &comment;

    /* find how long the tag is so we can check to see if
     * it's a block-level tag
     */
    for ( i=1; i < len && ithTMbr(p->text, i) != '>'
		       && ithTMbr(p->text, i) != '/'
		       && !isspace(ithTMbr(p->text, i)); ++i )
	;


    return mkd_search_tags(T(p->text)+1, i-1);
}


typedef struct _flo {
    Line *t;
    int i;
} FLO;

#define floindex(x) (x.i)


static unsigned int
flogetc(FLO *f)
{
    if ( f && f->t ) {
	if ( f->i < S(f->t->text) )
	    return (unsigned char)ithTMbr(f->t->text, f->i++);
	f->t = f->t->next;
	f->i = 0;
	return flogetc(f);
    }
    return EOF;
}


static void
splitline(Line *t, int cutpoint)
{
    if ( t && (cutpoint < S(t->text)) ) {
	Line *tmp = calloc(1, sizeof *tmp);

	tmp->next = t->next;
	t->next = tmp;

	SUFFIX(tmp->text, T(t->text)+cutpoint, S(t->text)-cutpoint);
	EXPAND(tmp->text) = 0;
	S(tmp->text)--;

	S(t->text) = cutpoint;
    }
}

#define UNCHECK(t) ((t)->is_checked = 0)

/*
 * walk a line, seeing if it's any of half a dozen interesting regular
 * types.
 */
static void
checkline(Line *l, mkd_flag_t *flags)
{
    int eol, i;
    int dashes = 0, spaces = 0,
	equals = 0, underscores = 0,
	stars = 0, tildes = 0, other = 0,
	backticks = 0;
    register int c,  first;

    l->is_checked = 1;
    l->kind = chk_text;
    l->is_fenced = 0;
    l->count = 0;

    if (l->dle >= 4) { l->kind=chk_code; return; }

    for ( eol = S(l->text); eol > l->dle && isspace(ithTMbr(l->text, eol-1)); --eol )
	;

    if ( is_flag_set(flags, MKD_FENCEDCODE) && !is_flag_set(flags, MKD_STRICT) ) {
	first = ithTMbr(l->text, l->dle);

	if ( first == '~' || first == '`' ) {
	    for ( i=l->dle; i<eol; i++ ) {
		c = ithTMbr(l->text, i);

		if ( (c != '~') && (c != '`') && (c != first) )
		    break;
		l->count++;
	    }

	    if ( l->count > 1 ) {
		l->kind = (first == '`' ? chk_backtick : chk_tilde);
		l->is_fenced = 1;
		return;
	    }
	}
    }

    for (i=l->dle; i<eol; i++) {

	if ( (c = ithTMbr(l->text, i)) != ' ' ) l->count++;

	switch (c) {
	case '-':  dashes = 1; break;
	case ' ':  spaces = 1; break;
	case '=':  equals = 1; break;
	case '_':  underscores = 1; break;
	case '*':  stars = 1; break;
	default:   other = 1; break;
	}
    }

    if ( dashes + equals + underscores + stars + tildes + backticks > 1 )
	return;

    if ( !other ) {
	if (underscores || stars )
	    l->kind = chk_hr;
	else if ( dashes )
	    l->kind = chk_dash;
	else if ( equals )
	    l->kind = chk_equal;
	return;
    }

    if ( tildes ) { l->kind = chk_tilde; }
    else if ( backticks ) { l->kind = chk_backtick; }
}



/* markdown only does special handling of comments if the comment end
 * is at the end of a line
 */
static Line *
commentblock(Paragraph *p, int *unclosed)
{
    Line *t, *ret;
    char *end;

       for ( t = p->text; t ; t = t->next) {
	   if ( end = strstr(T(t->text), "-->") ) {
	       if ( nextnonblank(t, 3 + (end - T(t->text))) < S(t->text) )
		   continue;
	       /*splitline(t, 3 + (end - T(t->text)) );*/
	       ret = t->next;
	       t->next = 0;
	       return ret;
	   }
    }

    *unclosed = 1;
    return t;

}


static Line *
htmlblock(Paragraph *p, struct kw *tag, int *unclosed)
{
    Line *ret;
    FLO f = { p->text, 0 };
    int c;
    int i, closing, depth=0;

    *unclosed = 0;

    if ( tag == &comment )
	return commentblock(p, unclosed);

    if ( tag->selfclose ) {
	ret = f.t->next;
	f.t->next = 0;
	return ret;
    }

    while ( (c = flogetc(&f)) != EOF ) {
	if ( c == '<' ) {
	    /* tag? */
	    c = flogetc(&f);
	    if ( c == '!' ) { /* comment? */
		if ( flogetc(&f) == '-' && flogetc(&f) == '-' ) {
		    /* yes */
		    while ( (c = flogetc(&f)) != EOF ) {
			if ( c == '-' && flogetc(&f) == '-'
				      && flogetc(&f) == '>')
			      /* consumed whole comment */
			      break;
		    }
		}
	    }
	    else { 
		if ( closing = (c == '/') ) c = flogetc(&f);

		for ( i=0; i < tag->size; i++, c=flogetc(&f) ) {
		    if ( tag->id[i] != toupper(c) )
			break;
		}

		if ( (i == tag->size) && !isalnum(c) ) {
		    depth = depth + (closing ? -1 : 1);
		    if ( depth == 0 ) {
			while ( c != EOF && c != '>' ) {
			    /* consume trailing gunk in close tag */
			    c = flogetc(&f);
			}
			if ( c == EOF )
			    break;
			if ( !f.t )
			    return 0;
			splitline(f.t, floindex(f));
			ret = f.t->next;
			f.t->next = 0;
			return ret;
		    }
		}
	    }
	}
    }
    *unclosed = 1;
    return 0;
}


/* footnotes look like ^<whitespace>{0,3}[stuff]: <content>$
 */
static int
isfootnote(Line *t)
{
    int i;

    if ( ( (i = t->dle) > 3) || (ithTMbr(t->text, i) != '[') )
	return 0;

    for ( ++i; i < S(t->text) ; ++i ) {
	register int c = ithTMbr(t->text, i);
	switch (c) {
	    case '[':
		return 0;
	    case ']':
		return ( ithTMbr(t->text, i+1) == ':' );
	}
    }
    return 0;
}


static inline int
isquote(Line *t)
{
    return (t->dle < 4 && ithTMbr(t->text, t->dle) == '>');
}


static inline int
iscode(Line *t)
{
    return (t->dle >= 4);
}


static inline int
ishr(Line *t, mkd_flag_t *flags)
{
    if ( ! (t->is_checked) )
	checkline(t, flags);

    if ( t->count > 2 )
	return t->kind == chk_hr || t->kind == chk_dash || t->kind == chk_equal;
    return 0;
}


static int
issetext(Line *t, int *htyp, mkd_flag_t *flags)
{
    Line *n;

    /* check for setext-style HEADER
     *                        ======
     */

    if ( (n = t->next) ) {
	if ( !(n->is_checked) )
	    checkline(n, flags);

	if ( n->kind == chk_dash || n->kind == chk_equal ) {
	    *htyp = SETEXT;
	    return 1;
	}
    }
    return 0;
}


static int
ishdr(Line *t, int *htyp, mkd_flag_t *flags)
{
    /* ANY leading `#`'s make this into an ETX header
     */
    if ( (t->dle == 0) && (S(t->text) > 1) && (ithTMbr(t->text, 0) == '#') ) {
	*htyp = ETX;
	return 1;
    }

    /* And if not, maybe it's a SETEXT header instead
     */
    return issetext(t, htyp, flags);
}


static inline int
end_of_block(Line *t, mkd_flag_t *flags)
{
    int dummy;

    if ( !t )
	return 0;

    return ( (S(t->text) <= t->dle) || ishr(t, flags) || ishdr(t, &dummy, flags) );
}


static Line*
is_discount_dt(Line *t, int *clip, mkd_flag_t *flags)
{
    if ( t && t->next
	   && (S(t->text) > 2)
	   && (t->dle == 0)
	   && (ithTMbr(t->text, 0) == '=')
	   && (ithTMbr(t->text, S(t->text)-1) == '=') ) {
	if ( t->next->dle >= 4 ) {
	    *clip = 4;
	    return t;
	}
	else
	    return is_discount_dt(t->next, clip, flags);
    }
    return 0;
}


static int
is_extra_dd(Line *t)
{
    return (t->dle < 4) && (ithTMbr(t->text, t->dle) == ':')
			&& isspace(ithTMbr(t->text, t->dle+1));
}


static Line*
is_extra_dt(Line *t, int *clip, mkd_flag_t* flags)
{
    if ( t && t->next && S(t->text) ) {
	Line *x;

	if ( iscode(t) || end_of_block(t, flags) )
	    return 0;

	if ( (x = skipempty(t->next)) && is_extra_dd(x) ) {
	    *clip = x->dle+2;
	    return t;
	}

	if ( x=is_extra_dt(t->next, clip, flags) )
	    return x;
    }
    return 0;
}

	
static Line *
isdefinition(Line *t, int *clip, int *list_type, mkd_flag_t *flags)
{
    Line *ret;
    
    if ( !is_flag_set(flags, MKD_STRICT) ) {
	if ( is_flag_set(flags, MKD_DLDISCOUNT) && (ret = is_discount_dt(t,clip,flags)) ) {
	    *list_type = 1;
	    return ret;
	}
	if ( is_flag_set(flags, MKD_DLEXTRA) && (ret = is_extra_dt(t,clip,flags)) ) {
	    *list_type = 2;
	    return ret;
	}
    }
    return 0;
}


static int
islist(Line *t, int *clip, mkd_flag_t *flags, int *list_type)
{
    int i, j;
    char *q;

    if ( end_of_block(t, flags) )
	return 0;

    if ( isdefinition(t,clip,list_type,flags) )
	return DL;
	
    if ( strchr("*-+", ithTMbr(t->text, t->dle)) && isspace(ithTMbr(t->text, t->dle+1)) ) {
	i = nextnonblank(t, t->dle+1);
	*clip = (i > 4) ? 4 : i;
	*list_type = UL;
	return is_flag_set(flags, MKD_EXPLICITLIST) ? UL : AL;
    }

    if ( (j = nextblank(t,t->dle)) > t->dle ) {
        if ( ithTMbr(t->text, j-1) == '.' ) {

	    if ( !(is_flag_set(flags, MKD_NOALPHALIST) || is_flag_set(flags, MKD_STRICT))
			  && (j == t->dle + 2)
			  && isalpha(ithTMbr(t->text, t->dle)) ) {
		j = nextnonblank(t,j);
		*clip = (j > 4) ? 4 : j;
		*list_type = AL;
		return AL;
	    }

	    strtoul(T(t->text)+t->dle, &q, 10);
	    if ( (q > T(t->text)+t->dle) && (q == T(t->text) + (j-1)) ) {
		j = nextnonblank(t,j);
		*clip = j;
		*list_type = OL;
		return AL;
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
	    pp->hnumber = (ithTMbr(p->next->text, 0) == '=') ? 1 : 2;

	    ret = p->next->next;
	    ___mkd_freeLine(p->next);
	    p->next = 0;
	    break;

    case ETX:
	    /* p->text is ###header###, so we need to trim off
	     * the leading and trailing `#`'s
	     */

	    for (i=0; (ithTMbr(p->text, i) == ithTMbr(p->text, 0)) && (i < S(p->text)-1); i++)
		;

	    pp->hnumber = (i > 6) ? 6 : i;;

	    while ( (i < S(p->text)) && isspace(ithTMbr(p->text, i)) )
		++i;

	    CLIP(p->text, 0, i);

	    for (i=S(p->text); (i > 0) && isspace(ithTMbr(p->text, i-1)); --i)
		;
	    S(p->text) = i;
	    ithTMbr(p->text, i) = 0;

	    UNCHECK(p);

	    for (j=S(p->text); (j > 1) && (ithTMbr(p->text, j-1) == '#'); --j)
		;

	    while ( j && isspace(ithTMbr(p->text, j-1)) )
		--j;

	    if ( j < S(p->text) ) {
		ithTMbr(p->text, j) = 0;
		S(p->text) = j;
	    }

	    ret = p->next;
	    p->next = 0;
	    break;
    }
    return ret;
}


static Line *
codeblock(Paragraph *p)
{
    Line *t = p->text, *r;

    for ( ; t; t = r ) {
	__mkd_trim_line(t,4);

	if ( !( (r = skipempty(t->next)) && iscode(r)) ) {
	    ___mkd_freeLineRange(t,r);
	    t->next = 0;
	    return r;
	}
    }
    return t;
}


static int
iscodefence(Line *r, int size, line_type kind, mkd_flag_t *flags)
{
    if ( !is_flag_set(flags, MKD_FENCEDCODE) || is_flag_set(flags, MKD_STRICT) )
	return 0;

    if ( !(r->is_checked) )
	checkline(r, flags);

    if ( !r->is_fenced )
	return 0;


    if ( kind )
	return (r->kind == kind) && (r->count >= size);
    else
	return (r->kind == chk_tilde || r->kind == chk_backtick) && (r->count >= size);
}


static Line *
fencedcodechunk(Line *first, mkd_flag_t *flags)
{
    Line *r, *q;

    /* don't allow zero-length code fences
    */
    if ( (first->next == 0) || iscodefence(first->next, first->count, first->kind, flags) ) {
	first->kind = chk_text;
	first->is_fenced = 0;
	if ( first->next) {
	    first->next->kind = chk_text;
	    first->next->is_fenced = 0;
	}
	return first->next;
    }

    /* find the closing fence, return a pointer to the Line
     * past the closing fence
     */
    for ( r = first->next; r; r = r->next ) {
	if ( iscodefence(r, first->count, first->kind, flags) ) {
	    if (S(first->text) - first->count > 0) {
		char *lang_attr = T(first->text) + first->count;

		while ( *lang_attr != 0 && *lang_attr == ' ' ) lang_attr++;

		if ( lang_attr && *lang_attr ) {
		    first->fence_class = strdup(lang_attr);
		}
	    }

	    for ( q = first; q && (q != r); q = q->next )
		q->is_fenced = 1;

	    S(first->text) = S(r->text) = 0;
	    r->is_fenced = 0;

	    return r->next;
	}
    }
    first->is_fenced = 0;
    first->kind = chk_text;
    return first;
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


static int
endoftextblock(Line *t, int toplevelblock, mkd_flag_t *flags)
{
    int z;

    if ( end_of_block(t, flags) || isquote(t) )
	return 1;

    /* HORRIBLE STANDARDS KLUDGES:
     * 1. non-toplevel paragraphs absorb adjacent code blocks
     * 2. Toplevel paragraphs eat absorb adjacent list items,
     *    but sublevel blocks behave properly.
     * (What this means is that we only need to check for code
     *  blocks at toplevel, and only check for list items at
     *  nested levels.)
     */
    return toplevelblock ? 0 : islist(t,&z,flags,&z);
}


static Line *
textblock(Paragraph *p, int toplevel, mkd_flag_t *flags)
{
    Line *t, *next;

    for ( t = p->text; t ; t = next ) {
	if ( iscodefence(t, t->count, 0, flags) )
	    next = fencedcodechunk(t, flags);
	else if ( ((next = t->next) == 0) || endoftextblock(next, toplevel, flags) ) {
	    p->align = centered(p->text, t);
	    t->next = 0;
	    return next;
	}
    }
    return t;
}


/* length of the id: or class: kind in a special div-not-quote block
 */
static int
szmarkerclass(char *p)
{
    if ( strncasecmp(p, "id:", 3) == 0 )
	return 3;
    if ( strncasecmp(p, "class:", 6) == 0 )
	return 6;
    return 0;
}


/*
 * check if the first line of a quoted block is the special div-not-quote
 * marker %[kind:]name%
 */

#define iscsschar(c) (isalpha(c) || (c == '-') || (c == '_') || (c == ' ') )

static int
isdivmarker(Line *p, int start, mkd_flag_t *flags)
{
    char *s;
    int last, i;

    if ( is_flag_set(flags, MKD_NODIVQUOTE) || is_flag_set(flags, MKD_STRICT) )
	return 0;

    start = nextnonblank(p, start);
    last= S(p->text) - (1 + start);
    s   = T(p->text) + start;

    if ( (last <= 0) || (*s != '%') || (s[last] != '%') )
	return 0;

    i = szmarkerclass(s+1);

    if ( !iscsschar(s[i+1]) )
	return 0;
    while ( ++i < last )
	if ( !(isdigit(s[i]) || iscsschar(s[i])) )
	    return 0;

    return 1;
}


/*
 * accumulate a blockquote.
 *
 * one sick horrible thing about blockquotes is that even though
 * it just takes ^> to start a quote, following lines, if quoted,
 * assume that the prefix is ``> ''.   This means that code needs
 * to be indented *5* spaces from the leading '>', but *4* spaces
 * from the start of the line.   This does not appear to be 
 * documented in the reference implementation, but it's the
 * way the markdown sample web form at Daring Fireball works.
 */
static Line *
quoteblock(Paragraph *p, mkd_flag_t *flags)
{
    Line *t, *q;
    int qp;

    for ( t = p->text; t ; t = q ) {
	if ( isquote(t) ) {
	    /* clip leading spaces */
	    for (qp = 0; ithTMbr(t->text, qp) != '>'; qp ++)
		/* assert: the first nonblank character on this line
		 * will be a >
		 */;
	    /* clip '>' */
	    qp++;
	    /* clip next space, if any */
	    if ( ithTMbr(t->text, qp) == ' ' )
		qp++;
	    __mkd_trim_line(t,qp);
	    checkline(t, flags);
	}

	q = skipempty(t->next);

	if ( (q == 0) || ((q != t->next) && (!isquote(q) || isdivmarker(q,1,flags))) ) {
	    ___mkd_freeLineRange(t, q);
	    t = q;
	    break;
	}
    }
    if ( isdivmarker(p->text,0,flags) ) {
	char *prefix = "class";
	int i;

	q = p->text;
	p->text = p->text->next;

	if ( (i = szmarkerclass(1+T(q->text))) == 3 )
	    /* and this would be an "%id:" prefix */
	    prefix="id";

	if ( p->ident = malloc(4+strlen(prefix)+S(q->text)) )
	    sprintf(p->ident, "%s=\"%.*s\"", prefix, S(q->text)-(i+2),
						     T(q->text)+(i+1) );

	___mkd_freeLine(q);
    }
    return t;
}


typedef int (*linefn)(Line *);


/*
 * pull in a list block.  A list block starts with a list marker and
 * runs until the next list marker, the next non-indented paragraph,
 * or EOF.   You do not have to indent nonblank lines after the list
 * marker, but multiple paragraphs need to start with a 4-space indent.
 */
static Line *
listitem(Paragraph *p, int indent, mkd_flag_t *flags, linefn check)
{
    Line *t, *q;
    int clip = indent;
    int z;
    int firstpara = 1;
    int ischeck;
#define CHECK_NOT 0
#define CHECK_NO 1
#define CHECK_YES 2

    for ( t = p->text; t ; t = q) {
	UNCHECK(t);

	__mkd_trim_line(t, clip);

	if ( firstpara && !(is_flag_set(flags, MKD_NORMAL_LISTITEM) || is_flag_set(flags, MKD_STRICT)) ) {
	    ischeck = CHECK_NOT;
	    if ( strncmp(T(t->text)+t->dle, "[ ]", 3) == 0 )
		ischeck = CHECK_NO;
	    else if ( strncasecmp(T(t->text)+t->dle, "[x]", 3) == 0 )
		ischeck = CHECK_YES;

	    if ( ischeck != CHECK_NOT ) {
		__mkd_trim_line(t, 3);
		p->para_flags |= GITHUB_CHECK;
		if ( ischeck == CHECK_YES )
		    p->para_flags |= IS_CHECKED;
	    }
	    firstpara = 0;
	}

        /* even though we had to trim a long leader off this item,
         * the indent for trailing paragraphs is still 4...
	 */
	if (indent > 4) {
	    indent = 4;
	}
	if ( (q = skipempty(t->next)) == 0 ) {
	    ___mkd_freeLineRange(t,q);
	    return 0;
	}

	/* after a blank line, the next block needs to start with a line
	 * that's indented 4(? -- reference implementation allows a 1
	 * character indent, but that has unfortunate side effects here)
	 * spaces, but after that the line doesn't need any indentation
	 */
	if ( q != t->next ) {
	    if (q->dle < indent) {
		q = t->next;
		t->next = 0;
		return q;
	    }
	    /* indent at least 2, and at most as
	     * as far as the initial line was indented. */
	    indent = clip ? clip : 2;
	}

	if ( (q->dle < indent) && (ishr(q,flags) || islist(q,&z,flags,&z)
					   || (check && (*check)(q)))
			       && !issetext(q,&z,flags) ) {
	    q = t->next;
	    t->next = 0;
	    return q;
	}

	clip = (q->dle > indent) ? indent : q->dle;
    }
    return t;
}


static Line *
definition_block(Paragraph *top, int clip, MMIOT *f, int kind)
{
    ParagraphRoot d = { 0, 0 };
    Paragraph *p;
    Line *q = top->text, *text = 0, *labels; 
    int z, para;

    while (( labels = q )) {

	if ( (q = isdefinition(labels, &z, &kind, &(f->flags))) == NULL )
	    break;

	if ( (text = skipempty(q->next)) == 0 )
	    break;

	if ( para = (text != q->next) )
	    ___mkd_freeLineRange(q, text);

	q->next = 0; 
	if ( kind == 1 /* discount dl */ )
	    for ( q = labels; q; q = q->next ) {
		CLIP(q->text, 0, 1);
		UNCHECK(q);
		S(q->text)--;
	    }

	do {
	    p = Pp(&d, text, LISTITEM);

	    text = listitem(p, clip, &(f->flags), (kind==2) ? is_extra_dd : 0);
	    p->down = compile(p->text, 0, f);
	    p->text = labels; labels = 0;

	    if ( para && p->down ) p->down->align = PARA;

	    if ( (q = skipempty(text)) == 0 )
		goto flee;

	    if ( para = (q != text) ) {
		Line anchor;

		anchor.next = text;
		___mkd_freeLineRange(&anchor,q);
		text = q;

	    }

	} while ( kind == 2 && is_extra_dd(q) );
    }
flee:
    top->text = 0;
    top->down = T(d);
    return text;
}


static Line *
enumerated_block(Paragraph *top, int clip, MMIOT *f, int list_class)
{
    ParagraphRoot d = { 0, 0 };
    Paragraph *p;
    Line *q = top->text, *text;
    int para = 0, z;

    while (( text = q )) {

	p = Pp(&d, text, LISTITEM);
	text = listitem(p, clip, &(f->flags), 0);

	p->down = compile(p->text, 0, f);
	p->text = 0;

	if ( para && p->down ) p->down->align = PARA;

	if ( (q = skipempty(text)) == 0
			     || islist(q, &clip, &(f->flags), &z) != list_class )
	    break;

	if ( para = (q != text) ) {
	    Line anchor;

	    anchor.next = text;
	    ___mkd_freeLineRange(&anchor, q);

	    if ( p->down ) p->down->align = PARA;
	}
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
 * eat lines for a markdown extra footnote
 */
static Line *
extrablock(Line *p)
{
    Line *np;

    while ( p && p->next ) {
	np = p->next;

	if ( np->dle < 4 && np->dle < S(np->text) ) {
	    p->next = 0;
	    return np;
	}
	__mkd_trim_line(np,4);
	p = np;
    }
    return 0;
}


/* return end of size string
 */
static int
findSizeEnd(char *t, int start)
{
    int end = endOfDigits(t, start);

    if ( end > start && t[end] == '%' )
	++end;

    return end;
}


/*
 * add a new (image or link) footnote to the footnote table
 */
static Line*
addfootnote(Line *p, MMIOT* f)
{
    int j, i;
    int c;
    Line *np = p->next;

    Footnote *foot = &EXPAND(f->footnotes->note);

    CREATE(foot->tag);
    CREATE(foot->link);
    CREATE(foot->title);
    foot->text = 0;
    foot->fn_flags = 0;
    DELETE(foot->height);
    DELETE(foot->width);

    /* keep the footnote label */
    for (j=i=p->dle+1; ithTMbr(p->text, j) != ']'; j++)
	EXPAND(foot->tag) = ithTMbr(p->text, j);
    EXPAND(foot->tag) = 0;
    S(foot->tag)--;

    /* consume the closing ]: */
    j = nextnonblank(p, j+2);

    if ( is_flag_set(&(f->flags), MKD_EXTRA_FOOTNOTE)
	     && !is_flag_set(&(f->flags), MKD_STRICT)
		     && (ithTMbr(foot->tag, 0) == '^') ) {
	/* markdown extra footnote: All indented lines past this point;
	 * the first line includes the footnote reference, so we need to
	 * snip that out as we go.
	 */
	foot->fn_flags |= EXTRA_FOOTNOTE;
	__mkd_trim_line(p,j);

	np = extrablock(p);

	foot->text = compile(p, 0, f);

	return np;
    }

    while ( (j < S(p->text)) && !isspace(ithTMbr(p->text, j)) )
	EXPAND(foot->link) = ithTMbr(p->text, j++);
    EXPAND(foot->link) = 0;
    S(foot->link)--;
    j = nextnonblank(p,j);

    if ( ithTMbr(p->text, j) == '=' ) {
	int end = findSizeEnd(T(p->text), ++j);
	if ( end > j ) {
	    Csprintf (&foot->width, "%.*s", end-j, T(p->text) + j);
	    j = end;
	}
	if ( ithTMbr(p->text, j) == 'x' ) {
	    end = findSizeEnd(T(p->text), ++j);
	    if ( end > j )
		Csprintf (&foot->height, "%.*s", end-j, T(p->text) + j);
	    j = end;
	}
	j = nextblank(p, j);
	j = nextnonblank(p,j);
    }


    if ( (j >= S(p->text)) && np && np->dle && tgood(ithTMbr(np->text, np->dle)) ) {
	___mkd_freeLine(p);
	p = np;
	np = p->next;
	j = p->dle;
    }

    if ( (c = tgood(ithTMbr(p->text, j))) ) {
	/* Try to take the rest of the line as a comment; read to
	 * EOL, then shrink the string back to before the final
	 * quote.
	 */
	++j;	/* skip leading quote */

	while ( j < S(p->text) )
	    EXPAND(foot->title) = ithTMbr(p->text, j++);

	while ( S(foot->title) && ithTMbr(foot->title, S(foot->title)-1) != c )
	    --S(foot->title);
	if ( S(foot->title) )	/* skip trailing quote */
	    --S(foot->title);
	EXPAND(foot->title) = 0;
	--S(foot->title);
    }

    ___mkd_freeLine(p);
    return np;
}


/*
 * allocate a paragraph header, link it to the
 * tail of the current document
 */
static Paragraph *
Pp(ParagraphRoot *d, Line *ptr, int typ)
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
    int blanks=0;

    for (; ptr && blankline(ptr); ptr = next, blanks++ ) {
	next = ptr->next;
	___mkd_freeLine(ptr);
    }
    if ( ptr ) *eaten = blanks;
    return ptr;
}


typedef ANCHOR(Line) Cache;

static void
uncache(Cache *cache, ParagraphRoot *d, MMIOT *f)
{
    Paragraph *p;

    if ( T(*cache) ) {
	E(*cache)->next = 0;
	p = Pp(d, 0, SOURCE);
	p->down = compile(T(*cache), 1, f);
	T(*cache) = E(*cache) = 0;
    }
}


/*
 * top-level compilation; break the document into
 * style, html, and source blocks with footnote links
 * weeded out.
 */
static Paragraph *
compile_document(Line *ptr, MMIOT *f)
{
    ParagraphRoot d = { 0, 0 };
    Cache source = { 0, 0 };
    Paragraph *p = 0;
    struct kw *tag;
    int eaten, unclosed;
    int previous_was_break = 1;

    while ( ptr ) {
	if ( !is_flag_set(&(f->flags), MKD_NOHTML) && (tag = isopentag(ptr)) ) {
	    int blocktype;
	    /* If we encounter a html/style block, compile and save all
	     * of the cached source BEFORE processing the html/style.
	     */
	    uncache(&source, &d, f);

	    if ( is_flag_set(&(f->flags), MKD_NOSTYLE) || is_flag_set(&(f->flags), MKD_STRICT) )
		blocktype = HTML;
	    else
		blocktype = strcmp(tag->id, "STYLE") == 0 ? STYLE : HTML;
	    p = Pp(&d, ptr, blocktype);
	    ptr = htmlblock(p, tag, &unclosed);
	    if ( unclosed ) {
		p->typ = SOURCE;
		p->down = compile(p->text, 1, f);
		p->text = 0;
	    }
	    previous_was_break = 1;
	}
	else if ( isfootnote(ptr) ) {
	    /* footnotes, like cats, sleep anywhere; pull them
	     * out of the input stream and file them away for
	     * later processing
	     */
	    ptr = consume(addfootnote(ptr, f), &eaten);
	    previous_was_break = 1;
	}
	else if (iscodefence(ptr, 2, 0, &(f->flags))) {
	    Line *more = ptr;

	    /* scoop up _everything_ in a fenced code block, including html & footnotes
	     */

	    ATTACH(source,more);

	    {   Cache checkpoint = source;
		while ( (more = more->next) && !iscodefence(more, ptr->count, ptr->kind, &(f->flags)) )
		    ATTACH(source,more);

		if ( more )
		    ptr = more;
		else {
		    source = checkpoint;
		    ptr = ptr->next;
		}
	    }
	}
	else {
	    /* source; cache it up to wait for eof or the
	     * next html/style block
	     */
	    ATTACH(source,ptr);
	    previous_was_break = blankline(ptr);
	    ptr = ptr->next;
	}
    }
    /* if there's any cached source at EOF, compile
     * it now.
     */
    uncache(&source, &d, f);

    /* if tables of contents are enabled, walk the document giving
     * all the headers unique labels
     */
    if ( is_flag_set(&(f->flags), MKD_TOC) && !is_flag_set(&(f->flags), MKD_STRICT) )
	___mkd_uniquify(&d, T(d));

    return T(d);
}


static int
first_nonblank_before(Line *j, int dle)
{
    return (j->dle < dle) ? j->dle : dle;
}


static int
actually_a_table(MMIOT *f, Line *pp)
{
    Line *r;
    int j;
    int c;

    /* tables need to be turned on */
    if ( is_flag_set(&(f->flags), MKD_NOTABLES) || is_flag_set(&(f->flags), MKD_STRICT) )
	return 0;

    /* tables need three lines */
    if ( !(pp && pp->next && pp->next->next) ) {
	return 0;
    }

    /* all lines must contain |'s */
    for (r = pp; r; r = r->next )
	if ( !(r->has_pipechar) ) {
	    return 0;
	}

    /* if the header has a leading |, all lines must have leading |'s */
    if ( ithTMbr(pp->text, pp->dle) == '|' ) {
	for ( r = pp; r; r = r->next )
	    if ( ithTMbr(r->text, first_nonblank_before(r,pp->dle)) != '|' ) {
		return 0;
	    }
    }

    /* second line must be only whitespace, -, |, or : */
    r = pp->next;

    for ( j=r->dle; j < S(r->text); ++j ) {
	c = ithTMbr(r->text, j);

	if ( !(isspace(c)||(c=='-')||(c==':')||(c=='|')) ) {
	    return 0;
	}
    }

    return 1;
}


/*
 * break a collection of markdown input into
 * blocks of lists, code, html, and text to
 * be marked up.
 */
static Paragraph *
compile(Line *ptr, int toplevel, MMIOT *f)
{
    ParagraphRoot d = { 0, 0 };
    Paragraph *p = 0;
    Line *r;
    int para = toplevel;
    int blocks = 0;
    int hdr_type, list_type, list_class, indent;

    ptr = consume(ptr, &para);

    while ( ptr ) {

	if ( iscode(ptr) ) {
	    p = Pp(&d, ptr, CODE);

	    if ( is_flag_set(&(f->flags), MKD_1_COMPAT) ) {
		/* HORRIBLE STANDARDS KLUDGE: the first line of every block
		 * has trailing whitespace trimmed off.
		 */
		___mkd_tidy(&p->text->text);
	    }

	    ptr = codeblock(p);
	}
	else if ( ishr(ptr, &(f->flags)) ) {
	    p = Pp(&d, 0, HR);
	    r = ptr;
	    ptr = ptr->next;
	    ___mkd_freeLine(r);
	}
	else if ( list_class = islist(ptr, &indent, &(f->flags), &list_type) ) {
	    if ( list_class == DL ) {
		p = Pp(&d, ptr, DL);
		ptr = definition_block(p, indent, f, list_type);
	    }
	    else {
		p = Pp(&d, ptr, list_type);
		ptr = enumerated_block(p, indent, f, list_class);
	    }
	}
	else if ( isquote(ptr) ) {
	    p = Pp(&d, ptr, QUOTE);
	    ptr = quoteblock(p, &(f->flags) );
	    p->down = compile(p->text, 1, f);
	    p->text = 0;
	}
	else if ( ishdr(ptr, &hdr_type, &(f->flags) ) ) {
	    p = Pp(&d, ptr, HDR);
	    ptr = headerblock(p, hdr_type);
	}
	else {
	    /* either markup or an html block element
	     */
	    struct kw *tag;
	    int unclosed = 1;

	    p = Pp(&d, ptr, MARKUP);	/* default to regular markup,
					 * then check if it's an html
					 * block.   If it IS an html
					 * block, htmlblock() will
					 * populate this paragraph &
					 * all we need to do is reset
					 * the paragraph type to HTML,
					 * otherwise the paragraph
					 * remains empty and ready for
					 * processing with textblock()
					 */

	    if ( !is_flag_set(&(f->flags), MKD_NOHTML) && (tag = isopentag(ptr)) ) {
		/* possibly an html block
		 */

		ptr = htmlblock(p, tag, &unclosed);
		if ( ! unclosed ) {
		    p->typ = HTML;
		}
	    }
	    if ( unclosed ) {
		ptr = textblock(p, toplevel, &(f->flags) );
		/* tables are a special kind of paragraph */
		if ( actually_a_table(f, p->text) )
		    p->typ = TABLE;
	    }
	}
	if ( (para||toplevel) && !p->align )
	    p->align = PARA;

	blocks++;
	para = toplevel || (blocks > 1);
	ptr = consume(ptr, &para);

	if ( para && !p->align )
	    p->align = PARA;

    }
    return T(d);
}


/*
 * the guts of the markdown() function, ripped out so I can do
 * debugging.
 */


/*
 * prepare and compile `text`, returning a Paragraph tree.
 */
int
mkd_compile(Document *doc, mkd_flag_t* flags)
{
    if ( !doc )
	return 0;

    if ( doc->compiled ) {
	if ( doc->dirty || DIFFERENT(flags, &doc->ctx->flags) ) {
	    doc->compiled = doc->dirty = 0;
	    if ( doc->code)
		___mkd_freeParagraph(doc->code);
	    if ( doc->ctx->footnotes )
		___mkd_freefootnotes(doc->ctx);
	}
	else
	    return 1;
    }

    doc->compiled = 1;
    memset(doc->ctx, 0, sizeof(MMIOT) );
    doc->ctx->ref_prefix= doc->ref_prefix;
    doc->ctx->cb        = &(doc->cb);
    if (flags)
	COPY_FLAGS(doc->ctx->flags, *flags);
    else
	mkd_init_flags(&doc->ctx->flags);
    CREATE(doc->ctx->in);
    doc->ctx->footnotes = malloc(sizeof doc->ctx->footnotes[0]);
    doc->ctx->footnotes->reference = 0;
    CREATE(doc->ctx->footnotes->note);


    mkd_initialize();

    doc->code = compile_document(T(doc->content), doc->ctx);
    qsort(T(doc->ctx->footnotes->note), S(doc->ctx->footnotes->note),
				sizeof ithTMbr(doc->ctx->footnotes->note, 0),
			           (stfu)__mkd_footsort);
    memset(&doc->content, 0, sizeof doc->content);
    return 1;
}

