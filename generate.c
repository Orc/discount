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
#include "amalloc.h"

typedef int (*stfu)(const void*,const void*);
typedef void (*spanhandler)(MMIOT*,int);

/* forward declarations */
static void text(MMIOT *f);
static Paragraph *display(Paragraph*, MMIOT*);

/* externals from markdown.c */
int __mkd_footsort(Footnote *, Footnote *);

/*
 * push text into the generator input buffer
 */
static void
push(char *bfr, int size, MMIOT *f)
{
    while ( size-- > 0 )
	EXPAND(f->in) = *bfr++;
}


/*
 * push a character into the generator input buffer
 */
static void 
pushc(char c, MMIOT *f)
{
    EXPAND(f->in) = c;
}


/* look <i> characters ahead of the cursor.
 */
static inline int
peek(MMIOT *f, int i)
{

    i += (f->isp-1);

    return (i >= 0) && (i < S(f->in)) ? (unsigned char)T(f->in)[i] : EOF;
}


/* pull a byte from the input buffer
 */
static inline unsigned int
pull(MMIOT *f)
{
    return ( f->isp < S(f->in) ) ? (unsigned char)T(f->in)[f->isp++] : EOF;
}


/* return a pointer to the current position in the input buffer.
 */
static inline char*
cursor(MMIOT *f)
{
    return T(f->in) + f->isp;
}


static inline int
isthisspace(MMIOT *f, int i)
{
    int c = peek(f, i);

    if ( c == EOF )
	return 1;
    if ( c & 0x80 )
	return 0;
    return isspace(c) || (c < ' ');
}


static inline int
isthisalnum(MMIOT *f, int i)
{
    int c = peek(f, i);

    return (c != EOF) && isalnum(c);
}


static inline int
isthisnonword(MMIOT *f, int i)
{
    return isthisspace(f, i) || ispunct(peek(f,i));
}


/* return/set the current cursor position
 * (when setting the current cursor position we also need to flush the
 * last character written cache)
 */
#define mmiotseek(f,x)	((f->isp = x), (f->last = 0))
#define mmiottell(f)	(f->isp)


/* move n characters forward ( or -n characters backward) in the input buffer.
 */
static void
shift(MMIOT *f, int i)
{
    if (f->isp + i >= 0 )
	f->isp += i;
}


/* Qchar()
 */
static void
Qchar(int c, MMIOT *f)
{
    block *cur;

    if ( S(f->Q) == 0 ) {
	cur = &EXPAND(f->Q);
	memset(cur, 0, sizeof *cur);
	cur->b_type = bTEXT;
    }
    else
	cur = &T(f->Q)[S(f->Q)-1];

    EXPAND(cur->b_text) = c;

}


/* Qstring()
 */
static void
Qstring(char *s, MMIOT *f)
{
    while (*s)
	Qchar(*s++, f);
}


/* Qwrite()
 */
static void
Qwrite(char *s, int size, MMIOT *f)
{
    while (size-- > 0)
	Qchar(*s++, f);
}


/* Qprintf()
 */
static void
Qprintf(MMIOT *f, char *fmt, ...)
{
    char bfr[80];
    va_list ptr;

    va_start(ptr,fmt);
    vsnprintf(bfr, sizeof bfr, fmt, ptr);
    va_end(ptr);
    Qstring(bfr, f);
}


/* Qanchor() prints out a suitable-for-id-tag version of a string
 */
static void
Qanchor(struct line *p, MMIOT *f)
{
    mkd_string_to_anchor(T(p->text), S(p->text),
			 (mkd_sta_function_t)Qchar, f, 1, f);
}


/* Qem()
 */
static void
Qem(MMIOT *f, char c, int count)
{
    block *p = &EXPAND(f->Q);

    memset(p, 0, sizeof *p);
    p->b_type = (c == '*') ? bSTAR : bUNDER;
    p->b_char = c;
    p->b_count = count;

    memset(&EXPAND(f->Q), 0, sizeof(block));
}


/* generate html from a markup fragment
 */
void
___mkd_reparse(char *bfr, int size, mkd_flag_t flags, MMIOT *f, char *esc)
{
    MMIOT sub;
    struct escaped e;

    ___mkd_initmmiot(&sub, f->footnotes);

    sub.flags = f->flags | flags;
    sub.cb = f->cb;
    sub.ref_prefix = f->ref_prefix;

    if ( esc ) {
	sub.esc = &e;
	e.up = f->esc;
	e.text = esc;
    }
    else
	sub.esc = f->esc;

    push(bfr, size, &sub);
    pushc(0, &sub);
    S(sub.in)--;

    text(&sub);
    ___mkd_emblock(&sub);

    Qwrite(T(sub.out), S(sub.out), f);
    /* inherit the last character printed from the reparsed
     * text;  this way superscripts can work when they're
     * applied to something embedded in a link
     */
    f->last = sub.last;

    ___mkd_freemmiot(&sub, f->footnotes);
}


/*
 * check the escape list for special cases
 */
static int
escaped(MMIOT *f, char c)
{
    struct escaped *thing = f->esc;

    while ( thing ) {
	if ( strchr(thing->text, c) )
	    return 1;
	thing = thing->up;
    }
    return 0;
}


/*
 * write out a url, escaping problematic characters
 */
static void
puturl(char *s, int size, MMIOT *f, int display)
{
    unsigned char c;

    while ( size-- > 0 ) {
	c = *s++;

	if ( c == '\\' && size-- > 0 ) {
	    c = *s++;

	    if ( !( ispunct(c) || isspace(c) ) )
		Qchar('\\', f);
	}

	if ( c == '&' )
	    Qstring("&amp;", f);
	else if ( c == '<' )
	    Qstring("&lt;", f);
	else if ( c == '"' )
	    Qstring("%22", f);
	else if ( isalnum(c) || ispunct(c) || (display && isspace(c)) )
	    Qchar(c, f);
	else if ( c == MKD_EOLN )	/* untokenize hard return */
	    Qstring("  ", f);
	else
	    Qprintf(f, "%%%02X", c);
    }
}


/* advance forward until the next character is not whitespace
 */
static int
eatspace(MMIOT *f)
{
    int c;

    for ( ; ((c=peek(f, 1)) != EOF) && isspace(c); pull(f) )
	;
    return c;
}


/* (match (a (nested (parenthetical (string.)))))
 */
static int
parenthetical(int in, int out, MMIOT *f)
{
    int size, indent, c;

    for ( indent=1,size=0; indent; size++ ) {
	if ( (c = pull(f)) == EOF )
	    return EOF;
	else if ( (c == '\\') && (peek(f,1) == out || peek(f,1) == in) ) {
	    ++size;
	    pull(f);
	}
	else if ( c == in )
	    ++indent;
	else if ( c == out )
	    --indent;
    }
    return size ? (size-1) : 0;
}


/* extract a []-delimited label from the input stream.
 */
static int
linkylabel(MMIOT *f, Cstring *res)
{
    char *ptr = cursor(f);
    int size;

    if ( (size = parenthetical('[',']',f)) != EOF ) {
	T(*res) = ptr;
	S(*res) = size;
	return 1;
    }
    return 0;
}


/* see if the quote-prefixed linky segment is actually a title.
 */
static int
linkytitle(MMIOT *f, char quote, Footnote *ref)
{
    int whence = mmiottell(f);
    char *title = cursor(f);
    char *e;
    register int c;

    while ( (c = pull(f)) != EOF ) {
	e = cursor(f);
	if ( c == quote ) {
	    if ( (c = eatspace(f)) == ')' ) {
		T(ref->title) = 1+title;
		S(ref->title) = (e-title)-2;
		return 1;
	    }
	}
    }
    mmiotseek(f, whence);
    return 0;
}


/* extract a =HHHxWWW size from the input stream
 */
static int
linkysize(MMIOT *f, Footnote *ref)
{
    int height=0, width=0;
    int whence = mmiottell(f);
    int c;

    if ( isspace(peek(f,0)) ) {
	pull(f);	/* eat '=' */

	for ( c = pull(f); isdigit(c); c = pull(f))
	    width = (width * 10) + (c - '0');

	if ( c == 'x' ) {
	    for ( c = pull(f); isdigit(c); c = pull(f))
		height = (height*10) + (c - '0');

	    if ( isspace(c) )
		c = eatspace(f);

	    if ( (c == ')') || ((c == '\'' || c == '"') && linkytitle(f, c, ref)) ) {
		ref->height = height;
		ref->width  = width;
		return 1;
	    }
	}
    }
    mmiotseek(f, whence);
    return 0;
}


/* extract a <...>-encased url from the input stream.
 * (markdown 1.0.2b8 compatibility; older versions
 * of markdown treated the < and > as syntactic
 * sugar that didn't have to be there.  1.0.2b8 
 * requires a closing >, and then falls into the
 * title or closing )
 */
static int
linkybroket(MMIOT *f, int image, Footnote *p)
{
    int c;
    int good = 0;

    T(p->link) = cursor(f);
    for ( S(p->link)=0; (c = pull(f)) != '>'; ++S(p->link) ) {
	/* pull in all input until a '>' is found, or die trying.
	 */
	if ( c == EOF )
	    return 0;
	else if ( (c == '\\') && ispunct(peek(f,2)) ) {
	    ++S(p->link);
	    pull(f);
	}
    }

    c = eatspace(f);

    /* next nonspace needs to be a title, a size, or )
     */
    if ( ( c == '\'' || c == '"' ) && linkytitle(f,c,p) )
	good=1;
    else if ( image && (c == '=') && linkysize(f,p) )
	good=1;
    else 
	good=( c == ')' );

    if ( good ) {
	if ( peek(f, 1) == ')' )
	    pull(f);

	___mkd_tidy(&p->link);
    }

    return good;
} /* linkybroket */


/* extract a (-prefixed url from the input stream.
 * the label is either of the format `<link>`, where I
 * extract until I find a >, or it is of the format
 * `text`, where I extract until I reach a ')', a quote,
 * or (if image) a '='
 */
static int
linkyurl(MMIOT *f, int image, Footnote *p)
{
    int c;
    int mayneedtotrim=0;

    if ( (c = eatspace(f)) == EOF )
	return 0;

    if ( c == '<' ) {
	pull(f);
	if ( !is_flag_set(f->flags, MKD_1_COMPAT) )
	    return linkybroket(f,image,p);
	mayneedtotrim=1;
    }

    T(p->link) = cursor(f);
    for ( S(p->link)=0; (c = peek(f,1)) != ')'; ++S(p->link) ) {
	if ( c == EOF )
	    return 0;
	else if ( (c == '"' || c == '\'') && linkytitle(f, c, p) )
	    break;
	else if ( image && (c == '=') && linkysize(f, p) )
	    break;
	else if ( (c == '\\') && ispunct(peek(f,2)) ) {
	    ++S(p->link);
	    pull(f);
	}
	pull(f);
    }
    if ( peek(f, 1) == ')' )
	pull(f);

    ___mkd_tidy(&p->link);

    if ( mayneedtotrim && (T(p->link)[S(p->link)-1] == '>') )
	--S(p->link);

    return 1;
}



/* prefixes for <automatic links>
 */
static struct _protocol {
    char *name;
    int   nlen;
} protocol[] = { 
#define _aprotocol(x)	{ x, (sizeof x)-1 }
    _aprotocol( "https:" ), 
    _aprotocol( "http:" ), 
    _aprotocol( "news:" ),
    _aprotocol( "ftp:" ), 
#undef _aprotocol
};
#define NRPROTOCOLS	(sizeof protocol / sizeof protocol[0])


static int
isautoprefix(char *text, int size)
{
    int i;
    struct _protocol *p;

    for (i=0, p=protocol; i < NRPROTOCOLS; i++, p++)
	if ( (size >= p->nlen) && strncasecmp(text, p->name, p->nlen) == 0 )
	    return 1;
    return 0;
}


/*
 * all the tag types that linkylinky can produce are
 * defined by this structure. 
 */
typedef struct linkytype {
    char      *pat;
    int      szpat;
    char *link_pfx;	/* tag prefix and link pointer  (eg: "<a href="\"" */
    char *link_sfx;	/* link suffix			(eg: "\""          */
    int        WxH;	/* this tag allows width x height arguments */
    char *text_pfx;	/* text prefix                  (eg: ">"           */
    char *text_sfx;	/* text suffix			(eg: "</a>"        */
    int      flags;	/* reparse flags */
    int      kind;	/* tag is url or something else? */
#define IS_URL	0x01
} linkytype;

static linkytype imaget = { 0, 0, "<img src=\"", "\"",
			     1, " alt=\"", "\" />", MKD_NOIMAGE|MKD_TAGTEXT, IS_URL };
static linkytype linkt  = { 0, 0, "<a href=\"", "\"",
                             0, ">", "</a>", MKD_NOLINKS, IS_URL };

/*
 * pseudo-protocols for [][];
 *
 * id: generates <a id="link">tag</a>
 * class: generates <span class="link">tag</span>
 * raw: just dump the link without any processing
 */
static linkytype specials[] = {
    { "id:", 3, "<span id=\"", "\"", 0, ">", "</span>", 0, 0 },
    { "raw:", 4, 0, 0, 0, 0, 0, MKD_NOHTML, 0 },
    { "lang:", 5, "<span lang=\"", "\"", 0, ">", "</span>", 0, 0 },
    { "abbr:", 5, "<abbr title=\"", "\"", 0, ">", "</abbr>", 0, 0 },
    { "class:", 6, "<span class=\"", "\"", 0, ">", "</span>", 0, 0 },
} ;

#define NR(x)	(sizeof x / sizeof x[0])

/* see if t contains one of our pseudo-protocols.
 */
static linkytype *
pseudo(Cstring t)
{
    int i;
    linkytype *r;

    for ( i=0, r=specials; i < NR(specials); i++,r++ ) {
	if ( (S(t) > r->szpat) && (strncasecmp(T(t), r->pat, r->szpat) == 0) )
	    return r;
    }
    return 0;
}


/* print out the start of an `img' or `a' tag, applying callbacks as needed.
 */
static void
printlinkyref(MMIOT *f, linkytype *tag, char *link, int size)
{
    char *edit;

    if ( is_flag_set(f->flags, IS_LABEL) )
	return;

    Qstring(tag->link_pfx, f);

    if ( tag->kind & IS_URL ) {
	if ( f->cb && f->cb->e_url && (edit = (*f->cb->e_url)(link, size, f->cb->e_data)) ) {
	    puturl(edit, strlen(edit), f, 0);
	    if ( f->cb->e_free ) (*f->cb->e_free)(edit, f->cb->e_data);
	}
	else
	    puturl(link + tag->szpat, size - tag->szpat, f, 0);
    }
    else
	___mkd_reparse(link + tag->szpat, size - tag->szpat, MKD_TAGTEXT, f, 0);

    Qstring(tag->link_sfx, f);

    if ( f->cb && f->cb->e_flags && (edit = (*f->cb->e_flags)(link, size, f->cb->e_data)) ) {
	Qchar(' ', f);
	Qstring(edit, f);
	if ( f->cb->e_free ) (*f->cb->e_free)(edit, f->cb->e_data);
    }
} /* printlinkyref */


/* helper function for php markdown extra footnotes; allow the user to
 * define a prefix tag instead of just `fn`
 */
static char *
p_or_nothing(p)
MMIOT *p;
{
    return p->ref_prefix ? p->ref_prefix : "fn";
}


/* php markdown extra/daring fireball style print footnotes
 */
static int
extra_linky(MMIOT *f, Cstring text, Footnote *ref)
{
    if ( ref->flags & REFERENCED )
	return 0;

    if ( f->flags & IS_LABEL )
    	___mkd_reparse(T(text), S(text), linkt.flags, f, 0);
    else {
	ref->flags |= REFERENCED;
	ref->refnumber = ++ f->footnotes->reference;
	Qprintf(f, "<sup id=\"%sref:%d\"><a href=\"#%s:%d\" rel=\"footnote\">%d</a></sup>",
		p_or_nothing(f), ref->refnumber,
		p_or_nothing(f), ref->refnumber, ref->refnumber);
    }
    return 1;
} /* extra_linky */



/* check a url (or url fragment to see that it begins with a known good
 * protocol (or no protocol at all)
 */
static int
safelink(Cstring link)
{
    char *p, *colon;

    if ( T(link) == 0 )	/* no link; safe */
	return 1;

    p = T(link);
    if ( (colon = memchr(p, ':', S(link))) == 0 )
	return 1; /* no protocol specified: safe */

    if ( !isalpha(*p) )	/* protocol/method is [alpha][alnum or '+.-'] */
	return 1;
    while ( ++p < colon )
	if ( !(isalnum(*p) || *p == '.' || *p == '+' || *p == '-') )
	    return 1;

    return isautoprefix(T(link), S(link));
}


/* print out a linky (or fail if it's Not Allowed)
 */
static int
linkyformat(MMIOT *f, Cstring text, int image, Footnote *ref)
{
    linkytype *tag;


    if ( image )
	tag = &imaget;
    else if ( tag = pseudo(ref->link) ) {
	if ( is_flag_set(f->flags, MKD_NO_EXT) || is_flag_set(f->flags, MKD_SAFELINK) )
	    return 0;
    }
    else if ( is_flag_set(f->flags, MKD_SAFELINK) && !safelink(ref->link) )
	/* if MKD_SAFELINK, only accept links that are local or
	 * a well-known protocol
	 */
	return 0;
    else
	tag = &linkt;

    if ( f->flags & tag->flags )
	return 0;

    if ( is_flag_set(f->flags, IS_LABEL) )
	___mkd_reparse(T(text), S(text), tag->flags, f, 0);
    else if ( tag->link_pfx ) {
	printlinkyref(f, tag, T(ref->link), S(ref->link));

	if ( tag->WxH ) {
	    if ( ref->height ) Qprintf(f," height=\"%d\"", ref->height);
	    if ( ref->width ) Qprintf(f, " width=\"%d\"", ref->width);
	}

	if ( S(ref->title) ) {
	    Qstring(" title=\"", f);
	    ___mkd_reparse(T(ref->title), S(ref->title), MKD_TAGTEXT, f, 0);
	    Qchar('"', f);
	}

	Qstring(tag->text_pfx, f);
	___mkd_reparse(T(text), S(text), tag->flags, f, 0);
	Qstring(tag->text_sfx, f);
    }
    else
	Qwrite(T(ref->link) + tag->szpat, S(ref->link) - tag->szpat, f);

    return 1;
} /* linkyformat */


/*
 * process embedded links and images
 */
static int
linkylinky(int image, MMIOT *f)
{
    int start = mmiottell(f);
    Cstring name;
    Footnote key, *ref;

    int status = 0;
    int extra_footnote = 0;

    CREATE(name);
    memset(&key, 0, sizeof key);

    if ( linkylabel(f, &name) ) {
	if ( peek(f,1) == '(' ) {
	    pull(f);
	    if ( linkyurl(f, image, &key) )
		status = linkyformat(f, name, image, &key);
	}
	else {
	    int goodlink, implicit_mark = mmiottell(f);

	    if ( isspace(peek(f,1)) )
		pull(f);

	    if ( peek(f,1) == '[' ) {
		pull(f);	/* consume leading '[' */
		goodlink = linkylabel(f, &key.tag);
	    }
	    else {
		/* new markdown implicit name syntax doesn't
		 * require a second []
		 */
		mmiotseek(f, implicit_mark);
		goodlink = !is_flag_set(f->flags, MKD_1_COMPAT);

		if ( is_flag_set(f->flags, MKD_EXTRA_FOOTNOTE) && (!image) && S(name) && T(name)[0] == '^' )
		    extra_footnote = 1;
	    }

	    if ( goodlink ) {
		if ( !S(key.tag) ) {
		    DELETE(key.tag);
		    T(key.tag) = T(name);
		    S(key.tag) = S(name);
		}

		if ( ref = bsearch(&key, T(f->footnotes->note),
					 S(f->footnotes->note),
					 sizeof key, (stfu)__mkd_footsort) ) {
		    if ( extra_footnote )
			status = extra_linky(f,name,ref);
		    else
			status = linkyformat(f, name, image, ref);
		}
	    }
	}
    }

    DELETE(name);
    ___mkd_freefootnote(&key);

    if ( status == 0 )
	mmiotseek(f, start);

    return status;
}


/* write a character to output, doing text escapes ( & -> &amp;,
 *                                          > -> &gt; < -> &lt; )
 */
static void
cputc(int c, MMIOT *f)
{
    switch (c) {
    case '&':   Qstring("&amp;", f); break;
    case '>':   Qstring("&gt;", f); break;
    case '<':   Qstring("&lt;", f); break;
    default :   Qchar(c, f); break;
    }
}


/*
 * convert an email address to a string of nonsense
 */
static void
mangle(char *s, int len, MMIOT *f)
{
    while ( len-- > 0 ) {
#if DEBIAN_GLITCH
	Qprintf(f, "&#%02d;", *((unsigned char*)(s++)) );
#else
	Qstring("&#", f);
	Qprintf(f, COINTOSS() ? "x%02x;" : "%02d;", *((unsigned char*)(s++)) );
#endif
    }
}


/* nrticks() -- count up a row of tick marks
 */
static int
nrticks(int offset, int tickchar, MMIOT *f)
{
    int  tick = 0;

    while ( peek(f, offset+tick) == tickchar ) tick++;

    return tick;
} /* nrticks */


/* matchticks() -- match a certain # of ticks, and if that fails
 *                 match the largest subset of those ticks.
 *
 *                 if a subset was matched, return the # of ticks
 *		   that were matched.
 */
static int
matchticks(MMIOT *f, int tickchar, int ticks, int *endticks)
{
    int size, count, c;
    int subsize=0, subtick=0;

    *endticks = ticks;
    for (size = 0; (c=peek(f,size+ticks)) != EOF; size ++) {
	if ( (c == tickchar) && ( count = nrticks(size+ticks,tickchar,f)) ) {
	    if ( count == ticks )
		return size;
	    else if ( count ) {
		if ( (count > subtick) && (count < ticks) ) {
		    subsize = size;
		    subtick = count;
		}
		size += count;
	    }
	}
    }
    if ( subsize ) {
	*endticks = subtick;
	return subsize;
    }
    return 0;
} /* matchticks */


/* code() -- write a string out as code. The only characters that have
 *           special meaning in a code block are * `<' and `&' , which
 *           are /always/ expanded to &lt; and &amp;
 */
static void
code(MMIOT *f, char *s, int length)
{
    int i,c;

    for ( i=0; i < length; i++ )
	if ( (c = s[i]) == MKD_EOLN)  /* expand back to 2 spaces */
	    Qstring("  ", f);
	else if ( c == '\\' && (i < length-1) && escaped(f, s[i+1]) )
	    cputc(s[++i], f);
	else
	    cputc(c, f);
} /* code */

/*  delspan() -- write out a chunk of text, blocking with <del>...</del>
 */
static void
delspan(MMIOT *f, int size)
{
    Qstring("<del>", f);
    ___mkd_reparse(cursor(f)-1, size, 0, f, 0);
    Qstring("</del>", f);
}

#ifdef TYPORA
/*  subspan() -- write out a chunk of text, blocking with <sub>...</sub>
 */
static void
subspan(MMIOT *f, int size)
{
    Qstring("<sub>", f);
    ___mkd_reparse(cursor(f)-1, size, 0, f, 0);
    Qstring("</sub>", f);
}


/*  supspan() -- write out a chunk of text, blocking with <sup>...</sup>
 */
static void
supspan(MMIOT *f, int size)
{
    Qstring("<sup>", f);
    ___mkd_reparse(cursor(f)-1, size, 0, f, 0);
    Qstring("</sup>", f);
}


/*  highlightspan() -- write out a chunk of text, blocking with <mark>...</mark>
 */
static void
highlightspan(MMIOT *f, int size)
{
    Qstring("<mark>", f);
    ___mkd_reparse(cursor(f)-1, size, 0, f, 0);
    Qstring("</mark>", f);
}
#endif



/*  codespan() -- write out a chunk of text as code, trimming one
 *                space off the front and/or back as appropriate.
 */
static void
codespan(MMIOT *f, int size)
{
    int i=0;

    if ( size > 1 && peek(f, size-1) == ' ' ) --size;
    if ( peek(f,i) == ' ' ) ++i, --size;

    Qstring("<code>", f);
    code(f, cursor(f)+(i-1), size);
    Qstring("</code>", f);
} /* codespan */


/* before letting a tag through, validate against
 * MKD_NOLINKS and MKD_NOIMAGE
 */
static int
forbidden_tag(MMIOT *f)
{
    int c = toupper(peek(f, 1));

    if ( is_flag_set(f->flags, MKD_NOHTML) )
	return 1;

    if ( c == 'A' && is_flag_set(f->flags, MKD_NOLINKS) && !isthisalnum(f,2) )
	return 1;
    if ( c == 'I' && is_flag_set(f->flags, MKD_NOIMAGE)
		  && strncasecmp(cursor(f)+1, "MG", 2) == 0
		  && !isthisalnum(f,4) )
	return 1;
    return 0;
}


/* Check a string to see if it looks like a mail address
 * "looks like a mail address" means alphanumeric + some
 * specials, then a `@`, then alphanumeric + some specials,
 * but with a `.`
 */
static int
maybe_address(char *p, int size)
{
    int ok = 0;

    for ( ;size && (isalnum(*p) || strchr("._-+*", *p)); ++p, --size)
	;

    if ( ! (size && *p == '@') )
	return 0;

    --size, ++p;

    if ( size && *p == '.' ) return 0;

    for ( ;size && (isalnum(*p) || strchr("._-+", *p)); ++p, --size )
	if ( *p == '.' && size > 1 ) ok = 1;

    return size ? 0 : ok;
}


/* The size-length token at cursor(f) is either a mailto:, an
 * implicit mailto:, one of the approved url protocols, or just
 * plain old text.   If it's a mailto: or an approved protocol,
 * linkify it, otherwise say "no"
 */
static int
process_possible_link(MMIOT *f, int size)
{
    int address= 0;
    int mailto = 0;
    char *text = cursor(f);

    if ( is_flag_set(f->flags, MKD_NOLINKS) ) return 0;

    if ( (size > 7) && strncasecmp(text, "mailto:", 7) == 0 ) {
	/* if it says it's a mailto, it's a mailto -- who am
	 * I to second-guess the user?
	 */
	address = 1;
	mailto = 7; 	/* 7 is the length of "mailto:"; we need this */
    }
    else 
	address = maybe_address(text, size);

    if ( address ) { 
	Qstring("<a href=\"", f);
	if ( !mailto ) {
	    /* supply a mailto: protocol if one wasn't attached */
	    mangle("mailto:", 7, f);
	}
	mangle(text, size, f);
	Qstring("\">", f);
	mangle(text+mailto, size-mailto, f);
	Qstring("</a>", f);
	return 1;
    }
    else if ( isautoprefix(text, size) ) {
	printlinkyref(f, &linkt, text, size);
	Qchar('>', f);
	puturl(text,size,f, 1);
	Qstring("</a>", f);
	return 1;
    }
    return 0;
} /* process_possible_link */


/*
 * check if a character is one of the things the reference implementation considers valid for starting
 * a html(ish) tag
 */
static inline int
is_a_strict_tag_prefix(int c)
{
    return isalpha(c) || (c == '/') || (c == '!') || (c == '$') || (c == '?');
}


/* a < may be just a regular character, the start of an embedded html
 * tag, or the start of an <automatic link>.    If it's an automatic
 * link, we also need to know if it's an email address because if it
 * is we need to mangle it in our futile attempt to cut down on the
 * spaminess of the rendered page.
 */
static int
maybe_tag_or_link(MMIOT *f)
{
    int c, size=0;

    if ( is_flag_set(f->flags, MKD_TAGTEXT) )
	return 0;

    c = peek(f, 1);


    if ( is_a_strict_tag_prefix(c) ) {
	/* By decree of Markdown.pl *this is a tag* and we want to absorb everything up
	 * to the next '>', unless interrupted by another '<' OR a '`', at which point
	 * we kick it back to the caller as plain old text.
	 */
	size=1;
	while ( (c=peek(f,size+1)) != '>' ) {
	    if ( c == EOF || c == '<' )
		return 0;
	    if ( is_flag_set(f->flags, MKD_STRICT) ) {
		if ( c == '`' )
		    return 0;
	    }
	    size++;
	}
    }

    if ( size > 0 ) {
	if ( process_possible_link(f, size) ) {
	    shift(f, size+1);
	    return 1;
	}
	else {
	    int i;

	    if ( forbidden_tag(f) )
		return 0;

	    for ( i=0; i <= size+1; i++ ) {
		c = peek(f,i);

		if ( (c == '&') && (i > 0) )
		    Qstring("&amp;", f);
		else
		    Qchar(c, f);
	    }

	    shift(f, size+1);
	    return 1;
	}
    }

    return 0;
}


/* autolinking means that all inline html is <a href'ified>.   A
 * autolink url is alphanumerics, slashes, periods, underscores,
 * the at sign, colon, and the % character.
 */
static int
maybe_autolink(MMIOT *f)
{
    register int c;
    int size;

    /* greedily scan forward for the end of a legitimate link.
     */
    for ( size=0; (c=peek(f, size+1)) != EOF; size++ ) {
	if ( c == '\\' ) {
	     if ( peek(f, size+2) != EOF )
		++size;
	}
	else if ( c & 0x80 )	/* HACK: ignore utf-8 extended characters */
	    continue;
	else if ( isspace(c) || strchr("'\"()[]{}<>`", c) || c == MKD_EOLN )
	    break;
    }

    if ( (size > 1) && process_possible_link(f, size) ) {
	shift(f, size);
	return 1;
    }
    return 0;
}


/* smartyquote code that's common for single and double quotes
 */
static int
smartyquote(int *flags, char typeofquote, MMIOT *f)
{
    int bit = (typeofquote == 's') ? 0x01 : 0x02;

    if ( bit & (*flags) ) {
	if ( isthisnonword(f,1) ) {
	    Qprintf(f, "&r%cquo;", typeofquote);
	    (*flags) &= ~bit;
	    return 1;
	}
    }
    else if ( isthisnonword(f,-1) && peek(f,1) != EOF ) {
	Qprintf(f, "&l%cquo;", typeofquote);
	(*flags) |= bit;
	return 1;
    }
    return 0;
}


static int
islike(MMIOT *f, char *s)
{
    int len;
    int i;

    if ( s[0] == '|' ) {
	if ( !isthisnonword(f, -1) )
	    return 0;
       ++s;
    }

    if ( !(len = strlen(s)) )
	return 0;

    if ( s[len-1] == '|' ) {
	if ( !isthisnonword(f,len-1) )
	    return 0;
	len--;
    }

    for (i=1; i < len; i++)
	if (tolower(peek(f,i)) != s[i])
	    return 0;
    return 1;
}


static struct smarties {
    char c0;
    char *pat;
    char *entity;
    int shift;
} smarties[] = {
    { '\'', "'s|",      "rsquo",  0 },
    { '\'', "'t|",      "rsquo",  0 },
    { '\'', "'re|",     "rsquo",  0 },
    { '\'', "'ll|",     "rsquo",  0 },
    { '\'', "'ve|",     "rsquo",  0 },
    { '\'', "'m|",      "rsquo",  0 },
    { '\'', "'d|",      "rsquo",  0 },
    { '-',  "---",      "mdash",  2 },
    { '-',  "--",       "ndash",  1 },
    { '.',  "...",      "hellip", 2 },
    { '.',  ". . .",    "hellip", 4 },
    { '(',  "(c)",      "copy",   2 },
    { '(',  "(r)",      "reg",    2 },
    { '(',  "(tm)",     "trade",  3 },
    { '3',  "|3/4|",    "frac34", 2 },
    { '3',  "|3/4ths|", "frac34", 2 },
    { '1',  "|1/2|",    "frac12", 2 },
    { '1',  "|1/4|",    "frac14", 2 },
    { '1',  "|1/4th|",  "frac14", 2 },
    { '&',  "&#0;",      0,       3 },
} ;
#define NRSMART ( sizeof smarties / sizeof smarties[0] )


/* Smarty-pants-style chrome for quotes, -, ellipses, and (r)(c)(tm)
 */
static int
smartypants(int c, int *flags, MMIOT *f)
{
    int i;

    if ( is_flag_set(f->flags, MKD_NOPANTS) 
      || is_flag_set(f->flags, MKD_TAGTEXT)
      || is_flag_set(f->flags, IS_LABEL) )
	return 0;

    for ( i=0; i < NRSMART; i++)
	if ( (c == smarties[i].c0) && islike(f, smarties[i].pat) ) {
	    if ( smarties[i].entity )
		Qprintf(f, "&%s;", smarties[i].entity);
	    shift(f, smarties[i].shift);
	    return 1;
	}

    switch (c) {
    case '<' :  return 0;
    case '\'':  if ( smartyquote(flags, 's', f) ) return 1;
		break;

    case '"':	if ( smartyquote(flags, 'd', f) ) return 1;
		break;

    case '`':   if ( peek(f, 1) == '`' ) {
		    int j = 2;

		    while ( (c=peek(f,j)) != EOF ) {
			if ( c == '\\' )
			    j += 2;
			else if ( c == '`' )
			    break;
			else if ( c == '\'' && peek(f, j+1) == '\'' ) {
			    Qstring("&ldquo;", f);
			    ___mkd_reparse(cursor(f)+1, j-2, 0, f, 0);
			    Qstring("&rdquo;", f);
			    shift(f,j+1);
			    return 1;
			}
			else ++j;
		    }

		}
		break;
    }
    return 0;
} /* smartypants */


/* process latex with arbitrary 2-character ( $$ .. $$, \[ .. \], \( .. \)
 * delimiters
 */
static int
mathhandler(MMIOT *f, int e1, int e2)
{
    int i = 0;

    while(peek(f, ++i) != EOF) {
        if (peek(f, i) == e1 && peek(f, i+1) == e2) {
	    cputc(peek(f,-1), f);
	    cputc(peek(f, 0), f);
	    while ( i-- > -1 )
		cputc(pull(f), f);
            return 1;
        }
    }
    return 0;
}


/* process a body of text encased in some sort of tick marks.   If it
 * works, generate the output and return 1, otherwise just return 0 and
 * let the caller figure it out.
 */
static int
tickhandler(MMIOT *f, int tickchar, int minticks, int allow_space, spanhandler spanner)
{
    int endticks, size;
    int tick = nrticks(0, tickchar, f);

    if ( !allow_space && isspace(peek(f,tick)) )
	return 0;

    if ( (tick >= minticks) && (size = matchticks(f,tickchar,tick,&endticks)) ) {
	if ( endticks < tick ) {
	    size += (tick - endticks);
	    tick = endticks;
	}

	shift(f, tick);
	(*spanner)(f,size);
	shift(f, size+tick-1);
	return 1;
    }
    return 0;
}

#define tag_text(f)	is_flag_set(f->flags, MKD_TAGTEXT)


static void
text(MMIOT *f)
{
    int c, j;
    int rep;
    int smartyflags = 0;

    while (1) {
        if ( is_flag_set(f->flags, MKD_AUTOLINK) && isalpha(peek(f,1)) && !tag_text(f) )
	    maybe_autolink(f);

        c = pull(f);

        if (c == EOF)
          break;

	if ( smartypants(c, &smartyflags, f) )
	    continue;
	switch (c) {
	case 0:     break;

	case MKD_EOLN:
		    Qstring(tag_text(f) ? "  " : "<br/>", f);
		    break;

	case '>':   if ( tag_text(f) )
			Qstring("&gt;", f);
		    else
			Qchar(c, f);
		    break;

	case '"':   if ( tag_text(f) )
			Qstring("&quot;", f);
		    else
			Qchar(c, f);
		    break;

	case '!':   if ( peek(f,1) == '[' ) {
			pull(f);
			if ( tag_text(f) || !linkylinky(1, f) )
			    Qstring("![", f);
		    }
		    else
			Qchar(c, f);
		    break;

	case '[':   if ( tag_text(f) || !linkylinky(0, f) )
			Qchar(c, f);
		    break;

#ifdef TYPORA
	case '=': if ( is_flag_set(f->flags, MKD_NOSUPERSCRIPT)
			 || is_flag_set(f->flags, MKD_STRICT)
			 || is_flag_set(f->flags, MKD_TAGTEXT)
			 || ! tickhandler(f,c,2,0, highlightspan))
			Qchar(c, f);
		    break;

	/* A^B^ -> A<sup>B</sup> */
	case '^':   if ( is_flag_set(f->flags, MKD_NOSUPERSCRIPT)
			 || is_flag_set(f->flags, MKD_STRICT)
			 || is_flag_set(f->flags, MKD_TAGTEXT)
			 || ! tickhandler(f,c,1,0, supspan))
			Qchar(c, f);
		    break;
#else /* !TYPORA */
	/* A^B -> A<sup>B</sup> */
	case '^':   if ( is_flag_set(f->flags, MKD_NOSUPERSCRIPT)
			    || is_flag_set(f->flags, MKD_STRICT)
			    || is_flag_set(f->flags, MKD_TAGTEXT)
			    || (f->last == 0)
			    || ((ispunct(f->last) || isspace(f->last))
						    && f->last != ')')
			    || isthisspace(f,1) )
			Qchar(c,f);
		    else {
			char *sup = cursor(f);
			int len = 0;

			if ( peek(f,1) == '(' ) {
			    int here = mmiottell(f);
			    pull(f);

			    if ( (len = parenthetical('(',')',f)) <= 0 ) {
				mmiotseek(f,here);
				Qchar(c, f);
				break;
			    }
			    sup++;
			}
			else {
			    while ( isthisalnum(f,1+len) )
				++len;
			    if ( !len ) {
				Qchar(c,f);
				break;
			    }
			    shift(f,len);
			}
			Qstring("<sup>",f);
			___mkd_reparse(sup, len, 0, f, "()");
			Qstring("</sup>", f);
		    }
		    break;
#endif /* TYPORA */
	case '_':
	/* Underscores don't count if they're in the middle of a word */
		    if ( !(is_flag_set(f->flags, MKD_NORELAXED) || is_flag_set(f->flags, MKD_STRICT))
				&& isthisalnum(f,-1) && isthisalnum(f,1) ) {
			Qchar(c, f);
			break;
		    }
	case '*':
	/* Underscores & stars don't count if they're out in the middle
	 * of whitespace */
		    if ( isthisspace(f,-1) && isthisspace(f,1) ) {
			Qchar(c, f);
			break;
		    }
		    /* else fall into the regular old emphasis case */
		    if ( tag_text(f) )
			Qchar(c, f);
		    else {
			for (rep = 1; peek(f,1) == c; pull(f) )
			    ++rep;
			Qem(f,c,rep);
		    }
		    break;

#ifdef TYPORA
#define ticktick(f,c) (tickhandler(f,c,2,0,delspan) || tickhandler(f,c,1,0,subspan))
#else
#define ticktick(f,c) tickhandler(f,c,2,0,delspan)
#endif

	case '~':   if ( is_flag_set(f->flags, MKD_NOSTRIKETHROUGH)
			 || is_flag_set(f->flags, MKD_STRICT)
			 || is_flag_set(f->flags, MKD_TAGTEXT)
			 || !ticktick(f,c) )
			Qchar(c, f);
		    break;

	case '`':   if ( tag_text(f) || !tickhandler(f,c,1,1,codespan) )
			Qchar(c, f);
		    break;

	case '\\':  switch ( c = pull(f) ) {
		    case '&':   Qstring("&amp;", f);
				break;
		    case '<':   c = peek(f,1);
				if ( (c == EOF) || isspace(c) )
				    Qstring("&lt;", f);
				else {
				    /* Markdown.pl does not escape <[nonwhite]
				     * sequences */
				    Qchar('\\', f);
				    shift(f, -1);
				}

				break;
		    case '^':   if ( is_flag_set(f->flags, MKD_STRICT)
					|| is_flag_set(f->flags, MKD_NOSUPERSCRIPT) ) {
				    Qchar('\\', f);
				    shift(f,-1);
				    break;
				}
				Qchar(c, f);
				break;

		    case ':': case '|':
				if ( is_flag_set(f->flags, MKD_NOTABLES) ) {
				    Qchar('\\', f);
				    shift(f,-1);
				    break;
				}
				Qchar(c, f);
				break;

		    case EOF:	Qchar('\\', f);
				break;

		    case '[':
		    case '(':   if ( is_flag_set(f->flags, MKD_LATEX)
				   && mathhandler(f, '\\', (c =='(')?')':']') )
				    break;
				/* else fall through to default */

		    default:    if ( escaped(f,c) ||
				     strchr(">#.-+{}]![*_\\()`", c) )
				    Qchar(c, f);
				else {
				    Qchar('\\', f);
				    shift(f, -1);
				}
				break;
		    }
		    break;

	case '<':   if ( !maybe_tag_or_link(f) ) {
			if ( is_flag_set(f->flags, MKD_STRICT) && is_a_strict_tag_prefix(peek(f,1)) )
			    Qchar(c, f);
			else
			    Qstring("&lt;", f);
		    }
		    break;

	case '&':   j = (peek(f,1) == '#' ) ? 2 : 1;
		    while ( isthisalnum(f,j) )
			++j;

		    if ( peek(f,j) != ';' )
			Qstring("&amp;", f);
		    else
			Qchar(c, f);
		    break;

	case '$':   if ( is_flag_set(f->flags, MKD_LATEX) && (peek(f, 1) == '$') ) {
			pull(f);
			if ( mathhandler(f, '$', '$') )
			    break;
			Qchar('$', f);
		    }
		    /* fall through to default */

	default:    f->last = c;
		    Qchar(c, f);
		    break;
	}
    }
    /* truncate the input string after we've finished processing it */
    S(f->in) = f->isp = 0;
} /* text */


/* print a header block
 */
static void
printheader(Paragraph *pp, MMIOT *f)
{
    if ( is_flag_set(f->flags, MKD_IDANCHOR) ) {
	Qprintf(f, "<h%d", pp->hnumber);
	if ( is_flag_set(f->flags, MKD_TOC) ) {
	    Qstring(" id=\"", f);
	    Qanchor(pp->text, f);
	    Qchar('"', f);
	}
	Qchar('>', f);
    } else {
	if ( is_flag_set(f->flags, MKD_TOC) ) {
	    Qstring("<a name=\"", f);
	    Qanchor(pp->text, f);
	    Qstring("\"></a>\n", f);
	}
	Qprintf(f, "<h%d>", pp->hnumber);
    }
    push(T(pp->text->text), S(pp->text->text), f);
    text(f);
    Qprintf(f, "</h%d>", pp->hnumber);
}


enum e_alignments { a_NONE, a_CENTER, a_LEFT, a_RIGHT };

static char* alignments[] = { "", " style=\"text-align:center;\"",
				  " style=\"text-align:left;\"",
				  " style=\"text-align:right;\"" };

typedef STRING(int) Istring;

static int
splat(Line *p, char *block, Istring align, int force, MMIOT *f)
{
    int first,
	idx = p->dle,
	colno = 0;


    ___mkd_tidy(&p->text);
    if ( T(p->text)[S(p->text)-1] == '|' )
	--S(p->text);

    Qstring("<tr>\n", f);
    while ( idx < S(p->text) ) {
	first = idx;
	if ( force && (colno >= S(align)-1) )
	    idx = S(p->text);
	else
	    while ( (idx < S(p->text)) && (T(p->text)[idx] != '|') ) {
		if ( T(p->text)[idx] == '\\' )
		    ++idx;
		++idx;
	    }

	Qprintf(f, "<%s%s>",
		   block,
		   alignments[ (colno < S(align)) ? T(align)[colno] : a_NONE ]);
	___mkd_reparse(T(p->text)+first, idx-first, 0, f, "|");
	Qprintf(f, "</%s>\n", block);
	idx++;
	colno++;
    }
    if ( force )
	while (colno < S(align) ) {
	    Qprintf(f, "<%s></%s>\n", block, block);
	    ++colno;
	}
    Qstring("</tr>\n", f);
    return colno;
}


static int
printtable(Paragraph *pp, MMIOT *f)
{
    /* header, dashes, then lines of content */

    Line *hdr, *dash, *body;
    Istring align;
    int hcols,start;
    char *p;
    enum e_alignments it;

    hdr = pp->text;
    dash= hdr->next;
    body= dash->next;

    if ( T(hdr->text)[hdr->dle] == '|' ) {
	/* trim leading pipe off all lines
	 */
	Line *r;
	for ( r = pp->text; r; r = r->next )
	    r->dle ++;
    }

    /* figure out cell alignments */

    CREATE(align);

    for (p=T(dash->text), start=dash->dle; start < S(dash->text); ) {
	char first, last;
	int end;

	last=first=0;
	for (end=start ; (end < S(dash->text)) && p[end] != '|'; ++ end ) {
	    if ( p[end] == '\\' )
		++ end;
	    else if ( !isspace(p[end]) ) {
		if ( !first) first = p[end];
		last = p[end];
	    }
	}
	it = ( first == ':' ) ? (( last == ':') ? a_CENTER : a_LEFT)
			      : (( last == ':') ? a_RIGHT : a_NONE );

	EXPAND(align) = it;
	start = 1+end;
    }

    Qstring("<table>\n", f);
    Qstring("<thead>\n", f);
    hcols = splat(hdr, "th", align, 0, f);
    Qstring("</thead>\n", f);

    if ( hcols < S(align) )
	S(align) = hcols;
    else
	while ( hcols > S(align) )
	    EXPAND(align) = a_NONE;

    Qstring("<tbody>\n", f);
    for ( ; body; body = body->next)
	splat(body, "td", align, 1, f);
    Qstring("</tbody>\n", f);
    Qstring("</table>\n", f);

    DELETE(align);
    return 1;
}


static int
printblock(Paragraph *pp, MMIOT *f)
{
    static char *Begin[] = { "", "<p>", "<p style=\"text-align:center;\">"  };
    static char *End[]   = { "", "</p>","</p>" };
    Line *t = pp->text;
    int align = pp->align;

    while (t) {
	if ( S(t->text) ) {
	    if ( t->next && S(t->text) > 2
			 && T(t->text)[S(t->text)-2] == ' '
			 && T(t->text)[S(t->text)-1] == ' ' ) {
		push(T(t->text), S(t->text)-2, f);
		pushc(MKD_EOLN, f);
		pushc('\n', f);
	    }
	    else {
		___mkd_tidy(&t->text);
		push(T(t->text), S(t->text), f);
		if ( t->next )
		    pushc('\n', f);
	    }
	}
	t = t->next;
    }
    Qstring(Begin[align], f);
    text(f);
    Qstring(End[align], f);
    return 1;
}


static void
printcode(Line *t, char *lang, MMIOT *f)
{
    int blanks;

    if ( f->cb->e_codefmt ) {
	/* external code block formatter;  copy the text into a buffer,
	 * call the formatter to style it, then dump that styled text
	 * directly to the queue
	 */
	char *text;
	char *fmt;
	int size, copy_p;
	Line *p;

	for (size=0, p = t; p; p = p->next )
	    size += 1+S(p->text);

	text = malloc(1+size);

	for ( copy_p = 0; t ; t = t->next ) {
	    memcpy(text+copy_p, T(t->text), S(t->text));
	    copy_p += S(t->text);
	    text[copy_p++] = '\n';
	}
	text[copy_p] = 0;

	fmt = (*(f->cb->e_codefmt))(text, copy_p, (lang && lang[0]) ? lang : 0);
	free(text);

	if ( fmt ) {
	    Qwrite(fmt, strlen(fmt), f);
	    if ( f->cb->e_free )
		(*(f->cb->e_free))(fmt, f->cb->e_data);
	    return;
	}
	/* otherwise the external formatter failed and we need to 
	 * fall back to the traditional codeblock format
	 */
    }

    Qstring("<pre><code", f);
    if (lang && lang[0]) {
      Qstring(" class=\"", f);
      Qstring(lang, f);
      Qstring("\"", f);
    }
    Qstring(">", f);
    for ( blanks = 0; t ; t = t->next ) {
	if ( S(t->text) > t->dle ) {
	    while ( blanks ) {
		Qchar('\n', f);
		--blanks;
	    }
	    code(f, T(t->text), S(t->text));
	    Qchar('\n', f);
	}
	else blanks++;
    }
    Qstring("</code></pre>", f);
}


static void
printhtml(Line *t, MMIOT *f)
{
    int blanks;

    for ( blanks=0; t ; t = t->next )
	if ( S(t->text) ) {
	    for ( ; blanks; --blanks ) 
		Qchar('\n', f);

	    Qwrite(T(t->text), S(t->text), f);
	    Qchar('\n', f);
	}
	else
	    blanks++;
}


static void
htmlify_paragraphs(Paragraph *p, MMIOT *f)
{
    ___mkd_emblock(f);

    while (( p = display(p, f) )) {
	___mkd_emblock(f);
	Qstring("\n\n", f);
    }
}


#ifdef GITHUB_CHECKBOX
static void
li_htmlify(Paragraph *p, char *arguments, mkd_flag_t flags, MMIOT *f)
{
    ___mkd_emblock(f);

    Qprintf(f, "<li");
    if ( arguments )
	Qprintf(f, " %s", arguments);
    if ( flags & GITHUB_CHECK )
	Qprintf(f, " class=\"github_checkbox\"");
    Qprintf(f, ">");
#if CHECKBOX_AS_INPUT
    if ( flags & GITHUB_CHECK ) {
	Qprintf(f, "<input disabled=\"\" type=\"checkbox\"");
	if ( flags & IS_CHECKED )
	    Qprintf(f, " checked=\"checked\"");
	Qprintf(f, "/>");
    }
#else
    if ( flags & GITHUB_CHECK )
	Qprintf(f, flags & IS_CHECKED ? "&#x2611;" : "&#x2610;");
#endif

    htmlify_paragraphs(p, f);

     Qprintf(f, "</li>");
    ___mkd_emblock(f);
}
#endif


static void
htmlify(Paragraph *p, char *block, char *arguments, MMIOT *f)
{
    ___mkd_emblock(f);
    if ( block )
	Qprintf(f, arguments ? "<%s %s>" : "<%s>", block, arguments);

    htmlify_paragraphs(p, f);

    if ( block )
	 Qprintf(f, "</%s>", block);
    ___mkd_emblock(f);
}


static void
definitionlist(Paragraph *p, MMIOT *f)
{
    Line *tag;

    if ( p ) {
	Qstring("<dl>\n", f);

	for ( ; p ; p = p->next) {
	    for ( tag = p->text; tag; tag = tag->next ) {
		Qstring("<dt>", f);
		___mkd_reparse(T(tag->text), S(tag->text), 0, f, 0);
		Qstring("</dt>\n", f);
	    }

	    htmlify(p->down, "dd", p->ident, f);
	    Qchar('\n', f);
	}

	Qstring("</dl>", f);
    }
}


static void
listdisplay(int typ, Paragraph *p, MMIOT* f)
{
    if ( p ) {
	Qprintf(f, "<%cl", (typ==UL)?'u':'o');
	if ( typ == AL )
	    Qprintf(f, " type=\"a\"");
	Qprintf(f, ">\n");

	for ( ; p ; p = p->next ) {
#ifdef GITHUB_CHECKBOX
	    li_htmlify(p->down, p->ident, p->flags, f);
#else
	    htmlify(p->down, "li", p->ident, f);
#endif
	    Qchar('\n', f);
	}

	Qprintf(f, "</%cl>\n", (typ==UL)?'u':'o');
    }
}


/* dump out a Paragraph in the desired manner
 */
static Paragraph*
display(Paragraph *p, MMIOT *f)
{
    if ( !p ) return 0;

    switch ( p->typ ) {
    case STYLE:
    case WHITESPACE:
	break;

    case HTML:
	printhtml(p->text, f);
	break;

    case CODE:
	printcode(p->text, p->lang, f);
	break;

    case QUOTE:
	htmlify(p->down, p->ident ? "div" : "blockquote", p->ident, f);
	break;

    case UL:
    case OL:
    case AL:
	listdisplay(p->typ, p->down, f);
	break;

    case DL:
	definitionlist(p->down, f);
	break;

    case HR:
	Qstring("<hr />", f);
	break;

    case HDR:
	printheader(p, f);
	break;

    case TABLE:
	printtable(p, f);
	break;

    case SOURCE:
	htmlify(p->down, 0, 0, f);
	break;

    default:
	printblock(p, f);
	break;
    }
    return p->next;
}


/* dump out a list of footnotes
 */
static void
mkd_extra_footnotes(MMIOT *m)
{
    int j, i;
    Footnote *t;

    if ( m->footnotes->reference == 0 )
	return;

    Csprintf(&m->out, "\n<div class=\"footnotes\">\n<hr/>\n<ol>\n");

    for ( i=1; i <= m->footnotes->reference; i++ ) {
	for ( j=0; j < S(m->footnotes->note); j++ ) {
	    t = &T(m->footnotes->note)[j];
	    if ( (t->refnumber == i) && (t->flags & REFERENCED) ) {
		Csprintf(&m->out, "<li id=\"%s:%d\">\n",
			    p_or_nothing(m), t->refnumber);
		htmlify(t->text, 0, 0, m);
		Csprintf(&m->out, "<a href=\"#%sref:%d\" rev=\"footnote\">&#8617;</a>",
			    p_or_nothing(m), t->refnumber);
		Csprintf(&m->out, "</li>\n");
	    }
	}
    }
    Csprintf(&m->out, "</ol>\n</div>\n");
}


/* return a pointer to the compiled markdown
 * document.
 */
int
mkd_document(Document *p, char **res)
{
    int size;

    if ( p && p->compiled ) {
	if ( ! p->html ) {
	    htmlify(p->code, 0, 0, p->ctx);
	    if ( is_flag_set(p->ctx->flags, MKD_EXTRA_FOOTNOTE) )
		mkd_extra_footnotes(p->ctx);
	    p->html = 1;
	    size = S(p->ctx->out);

	    if ( (size == 0) || T(p->ctx->out)[size-1] ) {
		/* Add a null byte at the end of the generated html,
		 * but pretend it doesn't exist.
		 */
		EXPAND(p->ctx->out) = 0;
		--S(p->ctx->out);
	    }
	}

	*res = T(p->ctx->out);
	return S(p->ctx->out);
    }
    return EOF;
}
