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

#include "cstring.h"
#include "markdown.h"

/* prefixes for <automatic links>
 */
static char *autoprefix[] = { "http://", "https://", "ftp://", "news://" };
#define SZAUTOPREFIX	(sizeof autoprefix / sizeof autoprefix[0])

/* block-level tags for passing html blocks through the blender
 */
static char *blocktags[] = { "ADDRESS", "BDO", "BLOCKQUOTE", "CENTER",
                      "DFN", "DIV", "H1", "H2", "H3", "H4",
		      "H5", "H6", "LISTING", "NOBR", "UL",
		      "P", "OL", "DL", "PLAINTEXT", "PRE",
		       "WBR", "XMP" };
#define SZTAGS	(sizeof blocktags / sizeof blocktags[0])

typedef int (*stfu)(const void*,const void*);


static void code(int, MMIOT*);
static void text(MMIOT *f);


/* case insensitive string sort (for qsort() and bsearch() of block tags)
 */
static int
casort(char **a, char **b)
{
    return strcasecmp(*a,*b);
}


/* case insensitive string sort for Footnote tags.
 */
static int
footsort(Footnote *a, Footnote *b)
{
    return strcasecmp(T(a->tag), T(b->tag));
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


static int
pull(MMIOT *f)
{
    if ( f->isp < S(f->in) ) return T(f->in)[f->isp++];

    S(f->in) = f->isp = 0;
    return EOF;
}


static char*
cursor(MMIOT *f)
{
    return T(f->in) + f->isp;
}


static void
shift(MMIOT *f, int i)
{
    if (f->isp + i >= 0 )
	f->isp += i;
}


static void
skipblankc(MMIOT *f)
{
    while ( isspace(peek(f,1)) )
	pull(f);
}


static void
reparse(char *bfr, int size, MMIOT *f)
{
    MMIOT sub;

    bzero(&sub, sizeof sub);
    sub.out = f->out;
    sub.isp = 0;
    sub.flags = f->flags|DENY_A;

    CREATE(sub.in);
    push(bfr, size, &sub);
    EXPAND(sub.in) = 0;
    S(sub.in)--;
    text(&sub);
    DELETE(sub.in);
}


static void
linkylinky(int image, MMIOT *f)
{
    char *label;
    int labelsize, size;
    int qc, c, i, j, indent;
    
    for ( indent = labelsize = 1; indent > 0; ++labelsize) {
	if ( (c=peek(f, labelsize)) == EOF )
	    return;
	else if ( c == '[' )
	    ++indent;
	else if ( c == ']' )
	    --indent;
    }
    
    label = cursor(f);

    for (j=labelsize; isspace(c = peek(f,j)); j++)
	;

    if ( (c != '[') && (c != '(') ) {
	fputc('[', f->out);
	return;
    }
    shift(f, j);

    fprintf(f->out, image ?  "<img" : "<a");
    
    switch (c) {
	char *link;
	char *tag;
	Footnote key, *ret;
    
    case '(':
	for ( size =1; (c=peek(f, size)) != ')' && !isspace(c); ++size)
	    if ( c == EOF )
		break;
	    
	link = cursor(f);
	shift(f, size-1);

	fprintf(f->out, " %s=\"%.*s\"", image ? "src" : "href", size-1, link);
	skipblankc(f);

	if ( image && (peek(f,1) == '=') ) {
	    int width, height;

	    pull(f);
	    for (i=0; isdigit(c=peek(f,i+1)) || (c == 'x'); i++)
		;

	    if ( i ) {
		if ( sscanf(cursor(f), "%dx%d", &width, &height) == 2 )
		    fprintf(f->out, " width=%d height=%d", width, height);
		shift(f, i);
	    }
	    skipblankc(f);
	}

	if ( (qc=peek(f,1)) == '"' || qc == '\'' ) {
	    fprintf(f->out, " title=\"");
	    pull(f);
	    while ( ((c=pull(f)) != EOF) && (c != qc) )
		fputc(c, f->out);
	    fputc('"', f->out);
	}
	if ( peek(f,1) == ')' ) pull(f);
	break;

    case '[':
	for ( size = 0; (c=peek(f, size+1)) != ']'; ++size)
	    if ( c == EOF )
		break;

	tag = alloca ( 2 + (size ? size : labelsize) );
	tag[0] = '[';

	if ( size ) {
	    memcpy(tag+1, cursor(f), size+1);
	    tag[size+2] = 0;
	    shift(f,size+1);
	}
	else  {
	    memcpy(tag+1, label, labelsize);
	    tag[labelsize] = 0;
	    pull(f);	/* discard the ']' from [] */
	}
	T(key.tag) = tag;
	ret = bsearch(&key, T(f->footnotes), S(f->footnotes),
	               sizeof key, (stfu)footsort);

	if ( ret ) {
	    if ( S(ret->link) )
		fprintf(f->out, " %s=\"%.*s\"", image ? "src" : "href", 
				S(ret->link), T(ret->link));
	    if ( S(ret->title) )
		fprintf(f->out, " title=\"%.*s\"",
				S(ret->title), T(ret->title));

	    if ( image && ret->height && ret->width )
		fprintf(f->out, " height=%d width=%d",
				ret->height, ret->width);
	}
	break;
    }

    if ( image )
	fprintf(f->out, " alt=\"%.*s\">", labelsize-2, label);
    else {
	fputc('>', f->out);
	reparse(label, labelsize-2, f);
	fprintf(f->out, "</a>");
    }
}


static void
mangle(unsigned char *s, int len, MMIOT *f)
{
    while ( len-- > 0 )
	fprintf(f->out, (random()&1) ? "&#x%02x;" : "&#%02d;", *s++);
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

    if ( maybetag ) {
	fprintf(f->out, forbidden_tag(f) ? "&lt;" : "<");
	return 1;
    }

    text = cursor(f);
    shift(f, size);

    for ( i=0; i < SZAUTOPREFIX; i++ )
	if ( strncasecmp(text, autoprefix[i], strlen(autoprefix[i])) == 0 ) {
	    fprintf(f->out, "&lt;<a href=\"%.*s\">%.*s</a>",
			    size, text, size, text);
	    return 1;
	}
    if ( maybeaddress ) {
	fprintf(f->out, "&lt;<a href=\"");
	mangle("mailto:", 7, f);
	mangle(text, size, f);
	fprintf(f->out,"\">");
	mangle(text, size, f);
	fprintf(f->out,"</a>");
	return 1;
    }

    shift(f, -size);
    return 0;
} /* maybe_tag_or_link */


static int
isthisblank(MMIOT *f, int i)
{
    int c = peek(f, i);

    return (c == EOF) || isspace(c) || ispunct(c);
}


/* smartyquote code that's common for single and double quotes
 */
static int
smartyquote(int *flags, char typeofquote, MMIOT *f)
{
    int bit = (typeofquote == 's') ? 0x01 : 0x02;

    if ( bit & (*flags) ) {
	if ( isthisblank(f,1) ) {
	    fprintf(f->out, "&r%cquo;", typeofquote);
	    (*flags) &= ~bit;
	    return 1;
	}
    }
    else if ( isthisblank(f,-1) && peek(f,1) != EOF ) {
	fprintf(f->out, "&l%cquo;", typeofquote);
	(*flags) |= bit;
	return 1;
    }
    return 0;
}


/* Smarty-pants-style chrome for quotes, -, ellipses, and (r)(c)(tm)
 */
static int
smartypants(int c, int *flags, MMIOT *f)
{
    if ( f->flags & DENY_SMARTY )
	return 0;

    switch (c) {
    case '\'':  if ( (c=toupper(peek(f,1)) == 'S' || c == 'T' )
		     && isthisblank(f, 2) ) {
		    /* 's or 't -> contraction or possessive ess.  Not
		     * smart enough
		     */
		    fprintf(f->out, "&rsquo;");
		    return 1;
		}
		if ( smartyquote(flags, 's', f) ) return 1;
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
			    fprintf(f->out, "&ldquo;");
			    reparse(cursor(f)+1, j-2, f);
			    fprintf(f->out, "&rdquo;");
			    shift(f,j+1);
			    return 1;
			}
			else ++j;
		    }

		}
		break;
		
    case '.':   if ( peek(f, 1) == '.' && peek(f, 2) == '.' ) {
		    fprintf(f->out, "&hellip;");
		    shift(f,2);
		    return 1;
		}
		break;

    case '-':   if ( peek(f, 1) == '-' ) {
		    fprintf(f->out, "&mdash;");
		    pull(f);
		    return 1;
		}
		else if ( isspace(peek(f,-1)) && isspace(peek(f,1)) ) {
		    fprintf(f->out, "&ndash;");
		    return 1;
		}
		break;

    case '(':   c = toupper(peek(f,1));
		if ( (c == 'C' || c == 'R') && (peek(f,2) == ')') ) {
		    fprintf(f->out, "&%s;", (c=='C') ? "copy" : "reg" );
		    shift(f,2);
		    return 1;
		}
		else if ( (c == 'T') && (toupper(peek(f,2)) == 'M')
		                     && (peek(f,3) == ')') ) {
		    fprintf(f->out, "&trade;");
		    shift(f,3);
		    return 1;
		}
		break;
    }
    return 0;
} /* smartypants */


static void
text(MMIOT *f)
{
    int c, j;
    int em = 0;
    int strong = 0;
    int smartyflags = 0;

    while ( (c = pull(f)) != EOF ) {
	if (smartypants(c, &smartyflags, f))
	    continue;
	switch (c) {
	case 0:     break;

/* Markdown transformations  (additional chrome is done at the end of this
 * switch)
 */
	case '!':   if ( (f->flags & DENY_IMG) || (peek(f,1) != '[') )
			fputc(c, f->out);
		    else {
			pull(f);
			linkylinky(1, f);
		    }
		    break;
	case '[':   if ( f->flags & DENY_A )
			fputc(c, f->out);
		    else
			linkylinky(0, f);
		    break;
	case '*':
	case '_':   if (peek(f,1) == c) {
			pull(f);
			if ( c == strong ) {
			    fprintf(f->out, "</strong>");
			    strong = 0;
			}
			else if ( strong == 0 ) {
			    fprintf(f->out, "<strong>");
			    strong = c;
			}
			else {
			    fputc(c, f->out);
			    fputc(c, f->out);
			}
		    }
		    else {
			if (c == em ) {
			    fprintf(f->out, "</em>");
			    em = 0;
			}
			else if ( em == 0 ) {
			    fprintf(f->out, "<em>");
			    em = c;
			}
			else
			    fputc(c, f->out);
		    }
		    break;
	
	case '`':   fprintf(f->out, "<code>");
		    if ( peek(f, 1) == '`' ) {
			pull(f);
			code(2, f);
		    }
		    else
			code(1, f);
		    fprintf(f->out, "</code>");
		    break;

	case '\\':  if ( (c = pull(f)) == '&' )
			fprintf(f->out, "&amp;");
		    else if ( c == '<' )
			fprintf(f->out, "&lt;");
		    else if ( c == '>' )
			fprintf(f->out, "&gt;");
		    else
			fputc( c ? c : '\\', f->out);
		    break;

	case '<':   maybe_tag_or_link(f) || fprintf(f->out, "&lt;");
		    break;

	case '&':   j = (peek(f,1) == '#' ) ? 2 : 1;
		    while ( isalnum(peek(f,j)) )
			++j;

		    if ( peek(f,j) != ';' )
			fprintf(f->out, "&amp;");
		    else
			fputc(c, f->out);
		    break;

	default:    fputc(c, f->out);
		    break;
	}
    }
    if ( em ) fputs("</em>", f->out);
    if ( strong ) fputs("</strong>", f->out);
} /* text */


/* the only characters that have special meaning in a code block are
 * `<' and `&' , which are /always/ expanded to &lt; and &amp;
 */
static void
code(int escape, MMIOT *f)
{
    int c;

    while ( (c = pull(f)) != EOF ) {
	switch (c) {
	case '&':   fprintf(f->out, "&amp;"); break;
	case '<':   fprintf(f->out, "&lt;"); break;
	case '`':   switch (escape) {
		    case 2: if ( peek(f,1) == '`' ) {
				shift(f,1);
		    case 1:     return;
		            }
		    }
	default:    fputc(c, f->out); break;
	}
    }
} /* code */


static char*
align(Paragraph *pp)
{
    if ( pp->align == CENTER )
	return "center";
    return 0;
}


/* print a header block (called by setext() or etx() [below])
 */
static void
header(char *p, int size, MMIOT *f, char *align, int depth)
{
    if ( align )
	 fprintf(f->out, "<%s>\n", align);
    fprintf(f->out, "<H%d>", depth);
    push(p, size, f); text(f);
    fprintf(f->out, "</H%d>\n", depth);
    if ( align )
	 fprintf(f->out, "</%s>\n", align);
}


/* setext header;  2 lines, second is ==== or -----
 */
static int
setext(Paragraph *pp, MMIOT *f)
{
    Line *p = pp->text;
    char *q;
    int i, H;

    if ( (p->next == 0) || p->next->next ) return 0;

    q = T(p->next->text);
    if ( (*q == '=') || (*q == '-') ) {
	for (i=1; i < S(p->next->text); i++)
	    if ( *q != T(p->next->text)[i] )
		return 0;

	H = (*q == '=') ? 1 : 2;

	header(T(p->text), S(p->text), f, align(pp), H);
	return 1;
    }
    return 0;
}


/* etx header;  # of #'s in front is the H level
 */
static int
etx(Paragraph *pp, MMIOT *f)
{
    Line *p = pp->text;
    int H = 0;
    int i, j;

    if ( p->next ) return 0;

    for ( i=0; T(p->text)[i] == '#'; ++i, ++H)
	;

    if ( !H ) return 0;

    i = nextnonblank(p, i);

    j = S(p->text)-1;

    while ( (j > i) && (T(p->text)[j] == '#') )
	--j;
    
    while ( (j > 1) && isspace(T(p->text)[j]) )
	--j;

    if ( j < i ) return 0;

    header(T(p->text)+i, 1+(j-i), f, align(pp), H);
    return 1;
}


static int
printblock(Paragraph *pp, MMIOT *f, int multiple)
{
    Line *t = pp->text;
    char *blocking = align(pp);

    if ( (blocking == 0) && multiple && pp->para )
	blocking = "p";
    
    while (t) {
	if ( S(t->text) ) {
	    if ( S(t->text) > 2 && T(t->text)[S(t->text)-2] == ' '
				&& T(t->text)[S(t->text)-1] == ' ') {
		push(T(t->text), S(t->text)-2, f);
		push("<br/>\n", 6, f);
	    }
	    else {
		push(T(t->text), S(t->text), f);
		push("\n", 1, f);
	    }
	}
	t = t->next;
    }
    if ( blocking ) fprintf(f->out, "<%s>\n", blocking);
    text(f);
    if ( blocking ) fprintf(f->out, "</%s>\n", blocking);
    return 1;
}


static void
printcode(Line *t, MMIOT *f)
{
    int blanks;

    for ( blanks = 0; t ; t = t->next )
	if ( S(t->text) ) {
	    while ( blanks ) {
		push("\n", 1, f);
		--blanks;
	    }
	    push(T(t->text), S(t->text), f);
	    push("\n", 1, f);
	}
	else blanks++;
    code(0, f);
}


static void
printhtml(Line *t, MMIOT *f)
{
    int blanks;
    
    for ( blanks=0; t ; t = t->next )
	if ( S(t->text) ) {
	    for ( ; blanks; --blanks ) 
		fputc('\n', f->out);

	    fwrite(T(t->text), S(t->text), 1, f->out);
	    fputc('\n', f->out);
	}
	else
	    blanks++;
}


static Paragraph *display(Paragraph*, MMIOT*, int);


static void
emit(Paragraph *p, MMIOT *f)
{
    int multiple = p && p->next;

    while (( p = display(p, f, multiple) ))
	;
}


static Paragraph*
listdisplay(Paragraph *p, MMIOT* f)
{
    int typ = p->typ;

    fprintf(f->out, "<%cl>\n", (typ==UL)?'u':'o');

    while ( p && (p->typ == typ) ) {
	fprintf(f->out, "<li>");
	emit(p->down, f);
	fprintf(f->out, "</li>\n");

	p = p->next;
    }

    fprintf(f->out, "</%cl>\n", (typ==UL)?'u':'o');
    return p;
}


/* dump out a Paragraph in the desired manner
 */
static Paragraph*
display(Paragraph *p, MMIOT *f, int multiple)
{
    if ( !p ) return;
    
    switch ( p->typ ) {
    case FORCED:
	break;

    case HTML:
	printhtml(p->text, f);
	break;
	
    case CODE:
	fprintf(f->out, "<code><pre>\n");
	printcode(p->text, f);
	fprintf(f->out, "</pre></code>\n");
	break;
	
    case QUOTE:
	fprintf(f->out, "<blockquote>\n");
	emit(p->down, f);
	fprintf(f->out, "</blockquote>\n");
	break;
	
    case UL:
    case OL:
	return listdisplay(p, f);

    case HR:
	fprintf(f->out, "<HR>\n");
	break;

    default:
	setext(p, f) || etx(p, f) || printblock(p, f, multiple);
	break;
    }
    return p->next;
}


static int
blankline(Line *p)
{
    return ! (p && (S(p->text) > p->dle) );
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
islist(Line *t, int *trim)
{
    int i, j;
    char *q;
    
    if ( blankline(t) || (t->dle > 3) )
	return 0;

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


static int
dashchar(char c)
{
    return (c == '*') || (c == '-') || (c == '_');
}


static int
ishr(Line *t)
{
    int i, count;
    char dash = 0;
    char c;

    for ( i = 0, count = 1; i < S(t->text); i++) {
	c = T(t->text)[i];
	if ( dashchar(c) ) {
	    if ( dash == c )
		++count;
	    else if ( dash == 0 )
		dash = c;
	    else
		return 0;
	}
	else if ( !isspace(c) )
	    return 0;
    }
    return (count >= 3);
}


static int
iscode(Line *t)
{
    return (t->dle >= 4);
}


static int
fancy(Line *t)
{
    int trim;

    return isquote(t) || iscode(t) || islist(t, &trim);
}


static Line *
codeblock(Paragraph *p)
{
    Line *t, *r, *ret;

    for ( t = p->text; t; t = r ) {
	CLIP(t->text,0,4);
	t->dle = mkd_firstnonblank(t);

	r = t->next;
	if ( r && S(r->text) && !iscode(r) ) {
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
    return LEFT;
}


static Line *
textblock(Paragraph *p)
{
    Line *t, *r;

    for ( t = p->text; t ; t = t->next ) {
	if ( (r = t->next) == 0 ) {
	    p->para = 2;
	    p->align = centered(p->text, t);
	    return 0;
	}
	else if ( fancy(r) ){
	    p->align = centered(p->text, t);
	    t->next = 0;
	    return r;
	}
	else if ( blankline(r) ) {
	    p->align = centered(p->text, t);
	    p->para = 2;
	    t->next = 0;
	    return r;
	}
    }
    return t;
}


static Line *
skipempty(Line *p)
{
    while ( p && (p->dle == S(p->text)) )
	p = p->next;
    return p;
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
    Line *t, *blank, *last = 0;
    int qp;

    for ( t = p->text; t ; t = t->next ) {
	if ( last && blankline(t) ) {
	    blank = last;
	    t = skipempty(t);

	    if ( !(t && isquote(t)) ) {
		blank->next = 0;
		return t;
	    }
	    last = blank;
	}
	else 
	    last = t;

	if ( (qp = quoteprefix(t)) > 0 ) {
	    CLIP(t->text, 0, qp);
	    t->dle = mkd_firstnonblank(t);
	}
    }
    return t;
}


static Line *
listblock(Paragraph *p, int trim)
{
    Line *t = p->text;
    Line *last = 0;

    do {
	if ( last ) {
	    if ( islist(t, &trim) ) {
		last->next = 0;
		return t;
	    }
	    else if ( blankline(t) ) {
		last = t;
		t = skipempty(t);

		if ( (t == 0) || (t->dle < 4) ) {
		    last->next = 0;
		    return t;
		}
	    }
	    else {
		last = t;
		t = t->next;
	    }
	}
	else {
	    last = t;
	    t = t->next;
	}

	CLIP(last->text,0,trim);
	last->dle = mkd_firstnonblank(last);

	if ( t ) trim = (t->dle < 4) ? t->dle : 4;
    } while ( t );
    return t;
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


typedef ANCHOR(Paragraph) Document;


/*
 * allocate a paragraph header, link it to the
 * tail of the current document
 */
static Paragraph *
Pp(Document *d, Line *ptr, int typ, int para)
{
    Paragraph *ret = calloc(sizeof *ret, 1);

    ret->text = ptr;
    ret->typ = typ;
    ret->para = para;
    ret->align = LEFT;

    return ATTACH(*d, ret);
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
    Paragraph *p;
    char *key;
    int list_type, indent;

    while (( ptr = skipempty(ptr) )) {
	if ( toplevel && (key = isopentag(ptr)) ) {
	    p = Pp(&d, ptr, HTML, 0);
	    ptr = htmlblock(p, key);
	}
	else if ( iscode(ptr) ) {
	    p = Pp(&d, ptr, CODE, 0);
	    ptr = codeblock(p);
	}
	else if ( ishr(ptr) ) {
	    p = Pp(&d, 0, HR, 0);
	    ptr = ptr->next;
	}
	else if (( list_type = islist(ptr, &indent) )) {
	    p = Pp(&d, ptr, list_type, 0);
	    ptr = listblock(p, indent);

	    p->down = compile(p->text, 0, f);
	}
	else if ( isquote(ptr) ) {
	    p = Pp(&d, ptr, QUOTE, 0);
	    ptr = quoteblock(p);
	    p->down = compile(p->text, 0, f);
	}
	else if ( toplevel && (isfootnote(ptr)) ) {
	    ptr = addfootnote(ptr, f);
	    continue;
	}
	else {
	    p = Pp(&d, ptr, MARKUP, toplevel);
	    ptr = textblock(p);
	}
    }
    return T(d);
}


static void
freeLine(Line *p)
{
    if (p->next) freeLine(p->next);
    DELETE(p->text);
    free(p);
}


static void
freeParagraph(Paragraph *p)
{
    if (p->next) freeParagraph(p->next);
    if (p->down) freeParagraph(p->down);
    else if (p->text) freeLine(p->text);
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
void
mkd_text(char *bfr, int size, FILE *output, int flags)
{
    MMIOT f;

    bzero(&f, sizeof f);
    f.out = output;
    CREATE(f.in);
    f.isp = 0;
    f.flags = flags;

    push(bfr, size, &f);
    EXPAND(f.in) = 0;
    S(f.in)--;
    text(&f);

    DELETE(f.in);
}


/* convert some markdown text to html
 */
int
markdown(Line *text, FILE *out, int flags)
{
    Paragraph *paragraph;
    MMIOT f;

    bzero(&f, sizeof f);
    f.out = out;
    f.flags = flags;
    CREATE(f.in);
    CREATE(f.footnotes);

    initmarkdown();

    paragraph = compile(text, 1, &f);
    qsort(T(f.footnotes), S(f.footnotes), sizeof T(f.footnotes)[0],
						    (stfu)footsort);
    emit(paragraph, &f);

    freefootnotes(&f);
    freeParagraph(paragraph);
    DELETE(f.in);
    return 0;
}
