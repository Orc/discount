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

static STRING(Footnote) footnotes;


typedef int (*stfu)(const void*,const void*);

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


static Cstring output;
static unsigned int csp = 0;

static void
push(char *t, int s)
{
    int i;

    for (i=0; i < s; i++)
	EXPAND(output) = t[i];
}


/* look <i> characters ahead of the cursor.
 */
static int
peek(int i)
{

    i += (csp-1);

    return (i >= 0) && (i < S(output)) ? T(output)[i] : EOF;
}


static int
pull()
{
    if ( csp < S(output) ) return T(output)[csp++];

    S(output) = csp = 0;
    return EOF;
}


static char*
cursor()
{
    return T(output) + csp;
}


static void
shift(int i)
{
    if ( (csp + i >= 0) && (csp + i < S(output)) )
	csp += i;
}


static void
skipblankc()
{
    while ( isspace(peek(1)) )
	pull();
}


static void
linkylinky(int image, FILE *out)
{
    char *label;
    int labelsize, size;
    int qc, c, i, j;
    
    for ( labelsize = 1; (c=peek(labelsize)) != ']'; ++labelsize)
	if ( c == EOF )
	    return;
    
    label = cursor();

    for (j=labelsize+1; isspace(c = peek(j)); j++)
	;

    if ( (c != '[') && (c != '(') ) {
	fputc('[', out);
	return;
    }
    shift(j);

    fprintf(out, image ?  "<img" : "<a");
    
    switch (c) {
	char *link;
	char *tag;
	Footnote key, *ret;
    
    case '(':
	for ( size =1; (c=peek(size)) != ')' && !isspace(c); ++size)
	    if ( c == EOF )
		break;
	    
	link = cursor();
	shift(size);

	fprintf(out, " %s=\"%.*s\"", image ? "src" : "href", size-1, link);
	skipblankc();

	if ( image && (peek(1) == '=') ) {
	    int width, height;

	    pull();
	    for (i=0; isdigit(c=peek(i+1)) || (c == 'x'); i++)
		;

	    if ( i ) {
		if ( sscanf(T(output)+csp, "%dx%d", &width, &height) == 2 )
		    fprintf(out, " width=%d height=%d", width, height);
		while ( i-- ) pull();
	    }
	    skipblankc();
	}

	if ( (qc=peek(1)) == '"' || qc == '\'' ) {
	    fprintf(out, " title=\"");
	    pull();
	    while ( ((c=pull()) != EOF) && (c != qc) )
		fputc(c, out);
	    fputc('"', out);
	}
	if ( peek(1) == ')' ) pull();
	break;

    case '[':
	for ( size = 0; (c=peek(size+1)) != ']'; ++size)
	    if ( c == EOF )
		break;

	tag = alloca ( 2 + (size ? size : labelsize) );
	tag[0] = '[';

	if ( size ) {
	    memcpy(tag+1, cursor(), size+1);
	    shift(size+1);
	}
	else  {
	    memcpy(tag+1, label, labelsize);
	    tag[labelsize+1] = 0;
	    pull();	/* discard the ']' from [] */
	}

	T(key.tag) = tag;
	ret = bsearch(&key, T(footnotes), S(footnotes),
	               sizeof key, (stfu)footsort);

	if ( ret ) {
	    if ( S(ret->link) )
		fprintf(out, " %s=\"%.*s\"", image ? "src" : "href", 
					S(ret->link), T(ret->link));
	    if ( S(ret->title) )
		fprintf(out, " title=\"%.*s\"", S(ret->title), T(ret->title));

	    if ( image && ret->height && ret->width )
		fprintf(out, " height=%d width=%d", ret->height, ret->width);
	}
	break;
    }
    fprintf(out, image ? " alt=\"%.*s\"" : ">%.*s</a>", labelsize-1, label);
}


static void
mangle(unsigned char *s, int len, FILE *out)
{
    while ( len-- > 0 )
	fprintf(out, (random()&1) ? "&#x%02x;" : "&#%02d;", *s++);
}


/* a < may be just a regular character, the start of an embedded html
 * tag, or the start of an <automatic link>.    If it's an automatic
 * link, we also need to know if it's an email address because if it
 * is we need to mangle it in our futile attempt to cut down on the
 * spaminess of the rendered page.
 */
static int
maybe_tag_or_link(FILE *out)
{
    char *text;
    int c, size, i;
    int maybetag=1, maybeaddress=0;

    for ( size=0; ((c = peek(size+1)) != '>') && !isspace(c); size++ ) {
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
	fputc('<', out);
	return 1;
    }

    text = cursor();
    shift(size);

    for ( i=0; i < SZAUTOPREFIX; i++ )
	if ( strncasecmp(text, autoprefix[i], strlen(autoprefix[i])) == 0 ) {
	    printf("&lt;<a href=\"%.*s\">%.*s</a>", size, text, size, text);
	    return 1;
	}
    if ( maybeaddress ) {
	fprintf(out, "&lt;<a href=\"");
	mangle("mailto:", 7, out);
	mangle(text, size, out);
	fprintf(out,"\">");
	mangle(text, size, out);
	fprintf(out,"</a>");
	return 1;
    }

    shift(-size);
    return 0;
} /* maybe_tag_or_link */


static int
isthisblank(i)
{
    int c = peek(i);

    return (c == EOF) || isspace(c) || ispunct(c);
}


/* Smarty-pants-style chrome for quotes, -, ellipses, and (r)(c)(tm)
 */
static int
smartypants(int c, int *flags, FILE *out)
{
    int squo, dquo;

    switch (c) {
    case '"':   dquo = 0x01 & (*flags);
		if ( isthisblank ( dquo ? 1 : -1 ) ) {
		    fprintf(out, "&%cdquo;", dquo ? 'r' : 'l' );
		    (*flags) ^= 0x01;
		    return 1;
		}
		break;

    case '\'':  squo = 0x02 & (*flags);
		if ( isthisblank( squo ? 1 : -1 ) ) {
		    fprintf(out, "&%csquo;", squo ? 'r' : 'l' );
		    (*flags) ^= 0x02;
		    return 1;
		}
		break;

    case '.':   if ( peek(1) == '.' && peek(2) == '.' ) {
		    fprintf(out,"&hellip;");
		    shift(2);
		    return 1;
		}
		break;

    case '-':   if ( peek(1) == '-' ) {
		    fprintf(out, "&mdash;");
		    pull();
		    return 1;
		}
		else if ( isspace(peek(-1)) && isspace(peek(1)) ) {
		    fprintf(out, "&ndash;");
		    return 1;
		}
		break;

    case '(':   c = toupper(peek(1));
		if ( (c == 'C' || c == 'R') && (peek(2) == ')') ) {
		    fprintf(out, "&%s;", (c=='C') ? "copy" : "reg" );
		    shift(2);
		    return 1;
		}
		else if ( (c == 'T') && (toupper(peek(2)) == 'M')
		                     && (peek(3) == ')') ) {
		    fprintf(out, "&trade;");
		    shift(3);
		    return 1;
		}
		break;
    }
    return 0;
} /* smartypants */


static void code(int, FILE*);


static void
text(FILE *out)
{
    int c, j;
    int em = 0;
    int strong = 0;
    int smartyflags = 0;

    while ( (c = pull()) != EOF ) {
	if (smartypants(c, &smartyflags, out))
	    continue;
	switch (c) {
	case 0:     break;

/* Markdown transformations  (additional chrome is done at the end of this
 * switch)
 */
	case '!':   if ( peek(1) == '[' ) {
			pull();
			linkylinky(1, out);
		    }
		    else
			fputc(c, out);
		    break;
	case '[':   linkylinky(0, out);
		    break;
	case '*':
	case '_':   if (peek(1) == c) {
			pull();
			if ( c == strong ) {
			    fprintf(out, "</strong>");
			    strong = 0;
			}
			else if ( strong == 0 ) {
			    fprintf(out, "<strong>");
			    strong = c;
			}
			else {
			    fputc(c, out);
			    fputc(c, out);
			}
		    }
		    else {
			if (c == em ) {
			    fprintf(out, "</em>");
			    em = 0;
			}
			else if ( em == 0 ) {
			    fprintf(out, "<em>");
			    em = c;
			}
			else
			    fputc(c, out);
		    }
		    break;
	
	case '`':   fprintf(out, "<code>");
		    if ( peek(1) == '`' ) {
			pull();
			code(2, out);
		    }
		    else
			code(1, out);
		    fprintf(out, "</code>");
		    break;

	case '\\':  if ( (c = pull()) == '&' )
			fprintf(out, "&amp;");
		    else if ( c == '<' )
			fprintf(out, "&lt;");
		    else
			fputc( c ? c : '\\', out);
		    break;

	case '<':   maybe_tag_or_link(out) || fprintf(out, "&lt;");
		    break;

	case '&':   j = (peek(1) == '#' ) ? 2 : 1;
		    while ( isalnum(peek(j)) )
			++j;

		    if ( peek(j) != ';' )
			fprintf(out, "&amp;");
		    else
			fputc(c, out);
		    break;

	default:    fputc(c, out);
		    break;
	}
    }
    if ( em ) fputs("</em>", out);
    if ( strong ) fputs("</strong>", out);
} /* text */


/* the only characters that have special meaning in a code block are
 * `<' and `&' , which are /always/ expanded to &lt; and &amp;
 */
static void
code(int escape, FILE *out)
{
    int c;

    while ( (c = pull()) != EOF ) {
	switch (c) {
	case '&':   fprintf(out, "&amp;"); break;
	case '<':   fprintf(out, "&lt;"); break;
	case '`':   switch (escape) {
		    case 2: if ( peek(1) == '`' ) {
				shift(1);
		    case 1:     return;
		            }
		    }
	default:    fputc(c, out); break;
	}
    }
} /* code */


/* setext header;  2 lines, second is ==== or -----
 */
static int
setext(Line *p, FILE *out)
{
    char *q;
    int i;

    if ( (p->next == 0) || p->next->next ) return 0;

    q = T(p->next->text);
    if ( (*q == '=') || (*q == '-') ) {
	for (i=1; i < S(p->next->text); i++)
	    if ( *q != T(p->next->text)[i] )
		return 0;

	i = (*q == '=') ? 1 : 2;

	fprintf(out, "<H%d>", i);
	push(T(p->text), S(p->text)); text(out);
	fprintf(out, "</H%d>\n", i);

	return 1;
    }
    return 0;
}


/* etx header;  # of #'s in front is the H level
 */
static int
etx(Line *p, FILE *out)
{
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

    fprintf(out, "<H%d>", H);
    push(T(p->text)+i, 1+(j-i)); text(out);
    fprintf(out, "</H%d>", H);
    return 1;
}


void
printblock(Line *t, FILE *out)
{
    while (t) {
	if ( S(t->text) ) {
	    if ( S(t->text) > 2 && T(t->text)[S(t->text)-2] == ' '
				&& T(t->text)[S(t->text)-1] == ' ') {
		push(T(t->text), S(t->text)-2);
		push("<br/>\n", 6);
	    }
	    else {
		push(T(t->text), S(t->text));
		push("\n", 1);
	    }
	}
	t = t->next;
    }
    text(out);
}


void
printcode(Line *t, FILE *out)
{
    int blanks;

    for ( blanks = 0; t ; t = t->next )
	if ( S(t->text) ) {
	    while ( blanks ) {
		push("\n", 1);
		--blanks;
	    }
	    push(T(t->text), S(t->text));
	    push("\n", 1);
	}
	else blanks++;
    code(0, out);
}


void
printhtml(Line *t, FILE *out)
{
    int blanks;
    
    for ( blanks=0; t ; t = t->next )
	if ( S(t->text) ) {
	    while ( blanks ) {
		fputc('\n', out);
		--blanks;
	    }
	    fwrite(T(t->text), S(t->text), 1, out);
	    fputc('\n', out);
	}
	else
	    blanks++;
}


static Paragraph *display(Paragraph*, FILE*, int);


static void
emit(Paragraph *p, FILE *out)
{
    int multiple = ( p->next != 0 );

    while (( p = display(p, out, multiple) ))
	;
}


static Paragraph*
listdisplay(Paragraph *p, FILE* out)
{
    int typ = p->typ;

    fprintf(out, "<%cl>\n", (typ==UL)?'u':'o');

    while ( p && (p->typ == typ) ) {
	fprintf(out, "<li>");
	emit(p->down, out);
	fprintf(out, "</li>\n");

	p = p->next;
    }

    fprintf(out, "</%cl>\n", (typ==UL)?'u':'o');
    return p;
}


/* dump out a Paragraph in the desired manner
 */
static Paragraph*
display(Paragraph *p, FILE *out, int multiple)
{
    char *pp = 0;

    switch ( p->typ ) {
    case FORCED:
	break;

    case HTML:
	printhtml(p->text, out);
	break;
	
    case CODE:
	fprintf(out, "<p><code><pre>");
	printcode(p->text, out);
	fprintf(out, "</pre></code></p>\n");
	break;
	
    case QUOTE:
	fprintf(out, "<blockquote>");
	emit(p->down, out);
	fprintf(out, "</blockquote>\n");
	break;
	
    case UL:
    case OL:
	return listdisplay(p, out);

    case HR:
	fprintf(out, "<HR>\n");
	break;

    default:
	if ( p->align == CENTER )
	    pp = "center";
	else if ( multiple && p->para )
	    pp = "p";

	if ( pp ) fprintf(out, "<%s>", pp);
	if ( !(setext(p->text, out)||etx(p->text, out)) )
	    printblock(p->text, out);
	if ( pp ) fprintf(out, "</%s>\n", pp);
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

	    if ( !isquote(t) ) {
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
addfootnote(Line *p)
{
    int j, i;
    int c;
    Line *p2 = p->next;

    Footnote *foot = &EXPAND(footnotes);
    
    CREATE(foot->tag);
    CREATE(foot->link);
    CREATE(foot->title);
    foot->height = foot->width = 0;

    for (j=i=p->dle; T(p->text)[j] != ']'; j++)
	EXPAND(foot->tag) = T(p->text)[j];

    EXPAND(foot->tag) = T(p->text)[j++];
    j = nextnonblank(p, j+1);

    while ( (j < S(p->text)) && !isspace(T(p->text)[j]) )
	EXPAND(foot->link) = T(p->text)[j++];
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

    if ( (c = tgood(T(p->text)[j])) )
	while ( (j < S(p->text)-1) && (T(p->text)[++j] != c) )
	    EXPAND(foot->title) = T(p->text)[j];

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
compile(Line *ptr, int toplevel)
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

	    p->down = compile(p->text, 0);
	}
	else if ( isquote(ptr) ) {
	    p = Pp(&d, ptr, QUOTE, 0);
	    ptr = quoteblock(p);
	    p->down = compile(p->text, 0);
	}
	else if ( toplevel && (isfootnote(ptr)) ) {
	    ptr = addfootnote(ptr);
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
    free(T(p->text));
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
freefootnotes()
{
    int i;

    for (i=0; i < S(footnotes); i++) {
	DELETE(T(footnotes)[i].tag);
	DELETE(T(footnotes)[i].link);
	DELETE(T(footnotes)[i].title);
    }
    S(footnotes) = 0;
}


static void
initmarkdown()
{
    static int init = 0;

    if ( init ) return;

    srandom((unsigned int)time(0));
    qsort(blocktags, SZTAGS, sizeof blocktags[0], (stfu)casort);
    CREATE(footnotes);
    CREATE(output);
    init = 1;
}


void
markdown(Line *text, FILE *out, int flags)
{
    Paragraph *paragraph;
    initmarkdown();

    paragraph = compile(text, 1);
    qsort(T(footnotes), S(footnotes), sizeof T(footnotes)[0], (stfu)footsort);

    emit(paragraph, out);

    freefootnotes();
    freeParagraph(paragraph);
}


#if PROGRAM
main(int argc, char **argv)
{

    if ( (argc > 1) && !freopen(argv[1], "r", stdin) ) {
	perror(argv[1]);
	exit(1);
    }
    markdown(in(stdin), stdout, 0);
    exit(0);
}
#endif
