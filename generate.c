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

/* prefixes for <automatic links>
 */
static char *autoprefix[] = { "http://", "https://", "ftp://", "news://" };
#define SZAUTOPREFIX	(sizeof autoprefix / sizeof autoprefix[0])

typedef int (*stfu)(const void*,const void*);


/* forward declarations */
static void code(int, MMIOT*);
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


/* look <i> characters ahead of the cursor.
 */
static int
peek(MMIOT *f, int i)
{

    i += (f->isp-1);

    return (i >= 0) && (i < S(f->in)) ? T(f->in)[i] : EOF;
}


/* pull a byte from the input buffer
 */
static int
pull(MMIOT *f)
{
    return ( f->isp < S(f->in) ) ? T(f->in)[f->isp++] : EOF;
}


/* return a pointer to the current position in the input buffer.
 */
static char*
cursor(MMIOT *f)
{
    return T(f->in) + f->isp;
}


/* return/set the current cursor position
 */
#define mmiotseek(f,x)	(f->isp = x)
#define mmiottell(f)	(f->isp)


/* move n characters forward ( or -n characters backward) in the input buffer.
 */
static void
shift(MMIOT *f, int i)
{
    if (f->isp + i >= 0 )
	f->isp += i;
}



/* write a character, possibly with xml escaping
 */
static void
oputc(unsigned char c, MMIOT *f)
{
    if ( f->flags & CDATA_OUTPUT ) {
	if ( c == '<' )
	    fputs("&lt;", f->out);
	else if ( c == '>' )
	    fputs("&gt;", f->out);
	else if ( c == '&' )
	    fputs("&amp;", f->out);
	else if ( c == '"' )
	    fputs("&quot;", f->out);
	else if ( c == '\'' )
	    fputs("&apos;", f->out);
	else if ( isascii(c) )
	    fputc(c, f->out);
    }
    else
	fputc(c, f->out);
}


/* write a string with oputc()
 */
static void
oputs(char *s, MMIOT *f)
{
    while ( *s )
	oputc(*s++, f);
}


/* write a block with oputc()
 */
static void
oputblk(char *s, int len, MMIOT *f)
{
    while ( len-- > 0 )
	oputc(*s++, f);
}


/* generate html from a markup fragment
 */
static void
reparse(char *bfr, int size, int flags, MMIOT *f)
{
    MMIOT sub;

    memcpy(&sub, f, sizeof sub);
    sub.isp = 0;
    sub.flags |= flags;

    CREATE(sub.in);
    push(bfr, size, &sub);
    EXPAND(sub.in) = 0;
    S(sub.in)--;
    text(&sub);
    DELETE(sub.in);
}


/*
 * write out a url, escaping problematic characters
 */
static void
puturl(char *s, int size, MMIOT *f)
{
    unsigned char c;

    while ( size-- > 0 ) {
	c = *s++;

	if ( c == '&' )
	    oputs("&amp;", f);
	else if ( c == '<' )
	    oputs("&lt;", f);
	else if ( isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/'
			     || c == '=' || c == '?' || c == ':' || c == '#' )
	    oputc(c, f);
	else
	    fprintf(f->out, "%%%02X", c);
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
	else if ( c == in )
	    ++indent;
	else if ( c == out )
	    --indent;
    }
    return size-1;
}


/* extract a []-delimited label from the input stream.
 */
static char *
linkylabel(MMIOT *f, int *sizep)
{
    char *ptr = cursor(f);

    if ( (*sizep = parenthetical('[',']',f)) != EOF )
	return ptr;
    return 0;
}


/* extract a (-prefixed url from the input stream.
 * the label is either of the format `<link>`, where I
 * extract until I find a >, or it is of the format
 * `text`, where I extract until I reach a ')' or
 * whitespace.
 */
static char*
linkyurl(MMIOT *f, int *sizep)
{
    int size = 0;
    char *ptr;
    int c;

    if ( (c = eatspace(f)) == EOF )
	return 0;

    ptr = cursor(f);

    if ( c == '<' ) {
	pull(f);
	ptr++;
	if ( (size = parenthetical('<', '>', f)) == EOF )
	    return 0;
    }
    else {
	for ( ; ((c=pull(f)) != ')') && !isspace(c); size++)
	    if ( c == EOF ) return 0;
	if ( c == ')' )
	    shift(f, -1);
    }
    *sizep = size;
    return ptr;
}


/* extract a =HHHxWWW size from the input stream
 */
static int
linkysize(MMIOT *f, int *heightp, int *widthp)
{
    int height=0, width=0;
    int c;

    *heightp = 0;
    *widthp = 0;

    if ( (c = eatspace(f)) != '=' ) 
	return (c != EOF);
    pull(f);	/* eat '=' */

    for ( c = pull(f); isdigit(c); c = pull(f))
	width = (width * 10) + (c - '0');

    if ( c == 'x' ) {
	for ( c = pull(f); isdigit(c); c = pull(f))
	    height = (height*10) + (c - '0');

	if ( c != EOF ) {
	    if ( !isspace(c) ) shift(f, -1);
	    *heightp = height;
	    *widthp = width;
	    return 1;
	}
    }
    return 0;
}


/* extract a )-terminated title from the input stream.
 */
static char*
linkytitle(MMIOT *f, int *sizep)
{
    int countq=0, qc, c, size;
    char *ret, *lastqc = 0;

    eatspace(f);
    if ( (qc=pull(f)) != '"' && qc != '\'' && qc != '(' )
	return 0;

    if ( qc == '(' ) qc = ')';

    for ( ret = cursor(f); (c = pull(f)) != EOF;  ) {
	if ( (c == ')') && countq ) {
	    size = (lastqc ? lastqc : cursor(f)) - ret;
	    *sizep = size-1;
	    return ret;
	}
	else if ( c == qc ) {
	    lastqc = cursor(f);
	    countq++;
	}
    }
    return 0;
}


/* look up (or construct) a footnote from the [xxx] link
 * at the head of the stream.
 */
static int
linkykey(int image, Footnote *val, MMIOT *f)
{
    Footnote *ret;
    Cstring mylabel;

    memset(val, 0, sizeof *val);

    if ( (T(val->tag) = linkylabel(f, &S(val->tag))) == 0 )
	return 0;

    eatspace(f);
    switch ( pull(f) ) {
    case '(':
	/* embedded link */
	if ( (T(val->link) = linkyurl(f,&S(val->link))) == 0 )
	    return 0;

	if ( image && !linkysize(f, &val->height, &val->width) )
	    return 0;

	T(val->title) = linkytitle(f, &S(val->title));

	return peek(f,0) == ')';

    case '[':
	/* footnote link */
	mylabel = val->tag;
	if ( (T(val->tag) = linkylabel(f, &S(val->tag))) == 0 )
	    return 0;

	if ( !S(val->tag) ) {
	    val->tag = mylabel;
	}

	ret = bsearch(val, T(f->footnotes), S(f->footnotes),
	               sizeof *val, (stfu)__mkd_footsort);

	if ( ret ) {
	    val->tag = mylabel;
	    val->link = ret->link;
	    val->title = ret->title;
	    val->height = ret->height;
	    val->width = ret->width;
	    return 1;
	}
    }
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
} linkytype;

static linkytype imaget = { 0, 0, "<img src=\"", "\"",
			     1, " alt=\"", "\" />", DENY_IMG|INSIDE_TAG };
static linkytype linkt  = { 0, 0, "<a href=\"", "\"",
                             0, ">", "</a>", DENY_A };

/*
 * pseudo-protocols for [][];
 *
 * id: generates <a id="link">tag</a>
 * class: generates <span class="link">tag</span>
 * raw: just dump the link without any processing
 */
static linkytype specials[] = {
    { "id:", 3, "<a id=\"", "\"", 0, ">", "</a>", 0 },
    { "class:", 6, "<span class=\"", "\"", 0, ">", "</span>", 0 },
    { "raw:", 4, 0, 0, 0, 0, 0, 0 },
} ;

#define NR(x)	(sizeof x / sizeof x[0])

/* see if t contains one of our pseudo-protocols.
 */
static linkytype *
extratag(Cstring t)
{
    int i;
    linkytype *r;

    for ( i=0; i < NR(specials); i++ ) {
	r = &specials[i];
	if ( (S(t) > r->szpat) && (strncasecmp(T(t), r->pat, r->szpat) == 0) )
	    return r;
    }
    return 0;
}


/*
 * process embedded links and images
 */
static int
linkylinky(int image, MMIOT *f)
{
    int start = mmiottell(f);
    Footnote link;
    linkytype *tag;

    if ( !(linkykey(image, &link, f) && S(link.tag))  ) {
	mmiotseek(f, start);
	return 0;
    }

    if ( image )
	tag = &imaget;
    else if ( (f->flags & NO_PSEUDO_PROTO) || (tag = extratag(link.link)) == 0 )
	tag = &linkt;

    if ( f->flags & tag-> flags ) {
	mmiotseek(f, start);
	return 0;
    }

    if ( tag->link_pfx ) {
	oputs(tag->link_pfx, f);
	puturl(T(link.link) + tag->szpat, S(link.link) - tag->szpat, f);
	oputs(tag->link_sfx, f);

	if ( tag->WxH && link.height && link.width ) {
	    oputs(" height=\"", f);
	    fprintf(f->out, "%d", link.height);
	    oputs("\" width=\"", f);
	    fprintf(f->out, "%d", link.width);
	    oputc('"', f);
	}

	if ( S(link.title) ) {
	    oputs(" title=\"", f);
	    reparse(T(link.title), S(link.title), INSIDE_TAG, f);
	    oputc('"', f);
	}

	oputs(tag->text_pfx, f);
	reparse(T(link.tag), S(link.tag), tag->flags, f);
	oputs(tag->text_sfx, f);
    }
    else
	oputblk(T(link.link) + tag->szpat, S(link.link) - tag->szpat, f);

    return 1;
}


/*
 * convert an email address to a string of nonsense
 */
static void
mangle(unsigned char *s, int len, MMIOT *f)
{
    while ( len-- > 0 ) {
	oputs("&#", f);
	fprintf(f->out, COINTOSS() ? "x%02x;" : "%02d;", *s++);
    }
}


/* before letting a tag through, validate against
 * DENY_A and DENY_IMG
 */
static int
forbidden_tag(MMIOT *f)
{
    int c = toupper(peek(f, 1));

    if ( c == 'A' && (f->flags & DENY_A) && !isalnum(peek(f,2)) )
	return 1;
    if ( c == 'I' && (f->flags & DENY_IMG)
		  && strncasecmp(cursor(f)+1, "MG", 2) == 0
		  && !isalnum(peek(f,4)) )
	return 1;
    return 0;
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
    char *text;
    int c, size, i;
    int maybetag=1, maybeaddress=0;

    if ( f->flags & INSIDE_TAG )
	return 0;

    for ( size=0; ((c = peek(f,size+1)) != '>') && !isspace(c); size++ ) {
	if ( ! (c == '/' || isalnum(c) || c == '~') )
	    maybetag=0;
	if ( c == '@' )
	    maybeaddress=1;
	else if ( c == EOF )
	    return 0;
    }

    if ( size == 0 )
	return 0;

    if ( maybetag  || (size >= 3 && strncmp(cursor(f), "!--", 3) == 0) ) {
	oputs(forbidden_tag(f) ? "&lt;" : "<", f);
	return 1;
    }

    if ( f->flags & DENY_A ) return 0;

    text = cursor(f);
    shift(f, size+1);

    for ( i=0; i < SZAUTOPREFIX; i++ )
	if ( strncasecmp(text, autoprefix[i], strlen(autoprefix[i])) == 0 ) {
	    oputs("<a href=\"", f);
	    puturl(text,size,f);
	    oputs("\">", f);
	    reparse(text, size, DENY_A, f);
	    oputs("</a>", f);
	    return 1;
	}
    if ( maybeaddress ) {

	if ( (size > 7) && strncasecmp(text, "mailto:", 7) == 0 ) {
	    /* if the address is in the form mailto:<foo>, strip
	     * off the mailto: part of the address so that it
	     * won't display funnily.
	     *
	     * the magic number '7' is the length of the word "mailto:"
	     */
	    text += 7;
	    size -= 7;
	}

	oputs("<a href=\"", f);
	mangle("mailto:", 7, f);
	mangle(text, size, f);
	oputs("\">", f);
	mangle(text, size, f);
	oputs("</a>", f);
	return 1;
    }

    shift(f, -(size+1));
    return 0;
} /* maybe_tag_or_link */


static int
isthisspace(MMIOT *f, int i)
{
    int c = peek(f, i);

    return isspace(c) || (c == EOF);
}


static int
isthisnonword(MMIOT *f, int i)
{
    return isthisspace(f, i) || ispunct(peek(f,i));
}


/* smartyquote code that's common for single and double quotes
 */
static int
smartyquote(int *flags, char typeofquote, MMIOT *f)
{
    int bit = (typeofquote == 's') ? 0x01 : 0x02;

    if ( bit & (*flags) ) {
	if ( isthisnonword(f,1) ) {
	    oputs("&r", f);
	    oputc(typeofquote, f);
	    oputs("quo;", f);
	    (*flags) &= ~bit;
	    return 1;
	}
    }
    else if ( isthisnonword(f,-1) && peek(f,1) != EOF ) {
	oputs("&l", f);
	oputc(typeofquote, f);
	oputs("quo;", f);
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

    if ( s[0] == '<' ) {
	if ( !isthisnonword(f, -1) )
	    return 0;
       ++s;
    }

    if ( !(len = strlen(s)) )
	return 0;

    if ( s[len-1] == '>' ) {
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
    { '\'', "'s>",      "rsquo",  0 },
    { '\'', "'t>",      "rsquo",  0 },
    { '-',  "--",       "mdash",  1 },
    { '-',  "<->",      "ndash",  0 },
    { '.',  "...",      "hellip", 2 },
    { '.',  ". . .",    "hellip", 4 },
    { '(',  "(c)",      "copy",   2 },
    { '(',  "(r)",      "reg",    2 },
    { '(',  "(tm)",     "trade",  3 },
    { '3',  "<3/4>",    "frac34", 2 },
    { '3',  "<3/4ths>", "frac34", 2 },
    { '1',  "<1/2>",    "frac12", 2 },
    { '1',  "<1/4>",    "frac14", 2 },
    { '1',  "<1/4th>",  "frac14", 2 },
    { '&',  "&#0;",      0,       3 },
} ;
#define NRSMART ( sizeof smarties / sizeof smarties[0] )


/* Smarty-pants-style chrome for quotes, -, ellipses, and (r)(c)(tm)
 */
static int
smartypants(int c, int *flags, MMIOT *f)
{
    int i;

    if ( f->flags & DENY_SMARTY )
	return 0;

    for ( i=0; i < NRSMART; i++)
	if ( (c == smarties[i].c0) && islike(f, smarties[i].pat) ) {
	    if ( smarties[i].entity ) {
		oputc('&', f);
		oputs(smarties[i].entity, f);
		oputc(';', f);
	    }
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
			    oputs("&ldquo;", f);
			    reparse(cursor(f)+1, j-2, 0, f);
			    oputs("&rdquo;", f);
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


#define tag_text(f)	(f->flags & INSIDE_TAG)


static void
text(MMIOT *f)
{
    int c, j;
    int em = 0;
    int strong = 0;
    int smartyflags = 0;

    while ( (c = pull(f)) != EOF ) {
	if ( smartypants(c, &smartyflags, f) )
	    continue;
	switch (c) {
	case 0:     break;

	case '>':   if ( tag_text(f) )
			oputs("&gt;", f);
		    else
			oputc(c, f);
		    break;

	case '"':   if ( tag_text(f) )
			oputs("&quot;", f);
		    else
			oputc(c, f);
		    break;
			
	case '!':   if ( peek(f,1) == '[' ) {
			pull(f);
			if ( tag_text(f) || !linkylinky(1, f) )
			    oputs("![", f);
		    }
		    else
			oputc(c, f);
		    break;
	case '[':   if ( tag_text(f) || !linkylinky(0, f) )
			oputc(c, f);
		    break;
	case '*':
	case '_':   if ( tag_text(f) )
			oputc(c, f);
		    else if (peek(f,1) == c) {
			pull(f);
			if ( c == strong ) {
			    oputs("</strong>", f);
			    strong = 0;
			}
			else if ( strong == 0 ) {
			    oputs("<strong>", f);
			    strong = c;
			}
			else {
			    oputc(c, f);
			    oputc(c, f);
			}
		    }
		    else if ( (isthisspace(f,-1) && isthisspace(f,1))
			   || (isalnum(peek(f,-1)) && isalnum(peek(f,1))) )
			oputc(c, f);
		    else {
			if (c == em ) {
			    oputs("</em>", f);
			    em = 0;
			}
			else if ( em == 0 ) {
			    oputs("<em>", f);
			    em = c;
			}
			else
			    oputc(c, f);
		    }
		    break;
	
	case '`':   if ( tag_text(f) )
			oputc(c, f);
		    else {
			oputs("<code>", f);
			if ( peek(f, 1) == '`' ) {
			    pull(f);
			    code(2, f);
			}
			else
			    code(1, f);
			oputs("</code>", f);
		    }
		    break;

	case '\\':  switch ( c = pull(f) ) {
		    case '&':   oputs("&amp;", f);
				break;
		    case '<':   oputs("&lt;", f);
				break;
		    case '\\':
		    case '>': case '#': case '.': case '-':
		    case '+': case '{': case '}': case ']':
		    case '(': case ')': case '"': case '\'':
		    case '!': case '[': case '*': case '_':
		    case '`':	oputc(c, f);
				break;
		    default:
				oputc('\\', f);
				if ( c != EOF )
				    shift(f,-1);
				break;
		    }
		    break;

	case '<':   if ( !maybe_tag_or_link(f) )
			oputs("&lt;", f);
		    break;

	case '&':   j = (peek(f,1) == '#' ) ? 2 : 1;
		    while ( isalnum(peek(f,j)) )
			++j;

		    if ( peek(f,j) != ';' )
			oputs("&amp;", f);
		    else
			oputc(c, f);
		    break;

	default:    oputc(c, f);
		    break;
	}
    }
    if ( em ) oputs("</em>", f);
    if ( strong ) oputs("</strong>", f);
} /* text */


static int
endofcode(int escape, int offset, MMIOT *f)
{
    switch (escape) {
    case 2: if ( peek(f, offset+1) == '`' ) {
		shift(f,1);
    case 1:     shift(f,offset);
		return 1;
	    }
    default:return 0;
    }
}


/* the only characters that have special meaning in a code block are
 * `<' and `&' , which are /always/ expanded to &lt; and &amp;
 */
static void
code(int escape, MMIOT *f)
{
    int c;

    if ( escape && (peek(f,1) == ' ') )
	shift(f,1);

    while ( (c = pull(f)) != EOF ) {
	switch (c) {
	case '&':   oputs("&amp;", f); break;

	case '>':   oputs("&gt;", f); break;

	case '<':   oputs("&lt;", f); break;

	case ' ':   if ( peek(f,1) == '`' && endofcode(escape,1, f) )
			return;
		    oputc(c, f);
		    break;

	case '`':   if ( endofcode(escape,0, f) )
			return;
		    oputc(c, f);
		    break;

	case '\\':  oputc(c, f);
		    if ( peek(f,1) == '>' || (c = pull(f)) == EOF )
			break;
		    oputc(c, f);
		    break;

	default:    oputc(c, f); break;
	}
    }
} /* code */


/* print a header block
 */
static void
printheader(Paragraph *pp, MMIOT *f)
{
    oputs("<h", f);
    fprintf(f->out, "%d", pp->hnumber);
    oputc('>', f);
    push(T(pp->text->text), S(pp->text->text), f);
    text(f);
    oputs("</h", f);
    fprintf(f->out, "%d", pp->hnumber);
    oputc('>', f);
}


static int
printblock(Paragraph *pp, MMIOT *f)
{
    Line *t = pp->text;
    static char *Begin[] = { "", "<p>", "<center>"  };
    static char *End[]   = { "", "</p>","</center>" };

    while (t) {
	if ( S(t->text) ) {
	    if ( S(t->text) > 2 && T(t->text)[S(t->text)-2] == ' '
				&& T(t->text)[S(t->text)-1] == ' ') {
		push(T(t->text), S(t->text)-2, f);
		push("<br/>\n", 6, f);
	    }
	    else {
		push(T(t->text), S(t->text), f);
		if ( t->next )
		    push("\n", 1, f);
	    }
	}
	t = t->next;
    }
    oputs(Begin[pp->align], f);
    text(f);
    oputs(End[pp->align], f);
    return 1;
}


static void
printcode(Line *t, MMIOT *f)
{
    int blanks;

    for ( blanks = 0; t ; t = t->next )
	if ( S(t->text) > t->dle ) {
	    while ( blanks ) {
		push("\n", 1, f);
		--blanks;
	    }
	    push(T(t->text), S(t->text), f);
	    push("\n", 1, f);
	}
	else blanks++;

    oputs("<pre><code>", f);
    code(0, f);
    oputs("</code></pre>", f);
}


static void
printhtml(Line *t, MMIOT *f)
{
    int blanks;
    
    for ( blanks=0; t ; t = t->next )
	if ( S(t->text) ) {
	    for ( ; blanks; --blanks ) 
		oputc('\n', f);

	    oputblk(T(t->text), S(t->text), f);
	    oputc('\n', f);
	}
	else
	    blanks++;
}


static void
emit(Paragraph *p, char *block, MMIOT *f)
{
    if ( block ) {
	oputc('<', f);
	oputs(block, f);
	oputc('>', f);
    }

    while (( p = display(p, f) ))
	oputs("\n\n", f);

    if ( block ) {
	oputs("</", f);
	oputs(block, f);
	oputc('>', f);
    }
}


#if DL_TAG_EXTENSION
static void
definitionlist(Paragraph *p, MMIOT *f)
{
    Line *tag;

    if ( p ) {
	oputs("<dl>\n", f);

	for ( ; p ; p = p->next) {
	    oputs("<dt>", f);
	    if (( tag = p->text ))
		reparse(T(tag->text), S(tag->text), 0, f);
	    oputs("</dt>\n", f);

	    emit(p->down, "dd", f);
	}

	oputs("</dl>", f);
    }
}
#endif


static void
listdisplay(int typ, Paragraph *p, MMIOT* f)
{
    if ( p ) {
	oputc('<', f);
	oputc((typ==UL)?'u':'o', f);
	oputs("l>\n", f);

	for ( ; p ; p = p->next ) {
	    emit(p->down, "li", f);
	    oputc('\n', f);
	}

	oputs("</", f);
	oputc((typ==UL)?'u':'o', f);
	oputs("l>", f);
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
	printcode(p->text, f);
	break;
	
    case QUOTE:
	emit(p->down, "blockquote", f);
	break;
	
    case UL:
    case OL:
	listdisplay(p->typ, p->down, f);
	break;

#if DL_TAG_EXTENSION
    case DL:
	definitionlist(p->down, f);
	break;
#endif

    case HR:
	oputs("<hr />", f);
	break;

    case HDR:
	printheader(p, f);
	break;

    default:
	printblock(p, f);
	break;
    }
    return p->next;
}


/*
 * dump out stylesheet sections.
 */
static int
stylesheets(Paragraph *p, FILE *f)
{
    Line* q;

    for ( ; p ; p = p->next ) {
	if ( p->typ == STYLE ) {
	    for ( q = p->text; q ; q = q->next )
		if ( fwrite(T(q->text), S(q->text), 1, f) == 1 )
		    putc('\n', f);
		else
		    return EOF;
	}
	if ( p->down && (stylesheets(p->down, f) == EOF) )
	    return EOF;
    }
    return 0;
}


/* public interface for emit()
 */
int
mkd_generatehtml(Document *p, FILE *f)
{
    if ( p->compiled ) {
	p->ctx->out = f;
	emit(p->code, 0, p->ctx);
	putc('\n', f);
	return 0;
    }
    return -1;
}


/*  public interface for reparse()
 */
int
mkd_text(char *bfr, int size, FILE *output, int flags)
{
    MMIOT f;

    memset(&f, 0, sizeof f);
    f.out = output;
    f.flags = flags & DENY_MASK;
    
    reparse(bfr, size, 0, &f);
    return 0;
}


/* dump any embedded styles
 */
int
mkd_style(Document *d, FILE *f)
{
    if ( d && d->compiled )
	return stylesheets(d->code, f);
    return EOF;
}

