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
    if ( f->isp < S(f->in) ) return T(f->in)[f->isp++];

    S(f->in) = f->isp = 0;
    return EOF;
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
unsigned
mmseek(MMIOT *f, int i)
{
    if ( i >= 0 && i < S(f->in) )
	f->isp = i;
    return f->isp;
}


/* move n characters forward ( or -n characters backward) in the input buffer.
 */
static void
shift(MMIOT *f, int i)
{
    if (f->isp + i >= 0 )
	f->isp += i;
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
puturl(char *s, int size, FILE *f)
{
    unsigned char c;

    for ( ; size ; --size, ++s ) {
	switch (c = *s) {
	case '<':   fputs("&lt;", f); break;
	case '>':   fputs("&gt;", f); break;
	case '&':   fputs("&amp;", f); break;
	default:    if ( c == '"' || c == ' ' || !isprint(c) )
			fprintf(f, "%%%02x", c);
		    else
			fputc(c, f);
		    break;
	}
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


/* extract a []-delimited label from the input stream.
 */
static char *
linkylabel(MMIOT *f, int *sizep)
{
    int c, size, indent;
    char *ptr = cursor(f);

    for ( indent=1,size=0; indent > 0; size++ ) {
	if ( (c = pull(f)) == EOF )
	    return 0;
	else if ( c == '[' )
	    ++indent;
	else if ( c == ']' )
	    --indent;
    }
    *sizep = size-1;
    return ptr;
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
	ptr++;
	for ( pull(f);  (c=pull(f)) != '>'; size++)
	    if ( c == EOF ) return 0;
    }
    else {
	for ( ; ((c=pull(f)) != ')') && !isspace(c); size++)
	    if ( c == EOF ) return 0;
    }
    if ( c == ')' ) shift(f, -1);
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
	height = (height*10) + (c - '0');

    if ( c == 'x' ) {
	for ( c = pull(f); isdigit(c); c = pull(f))
	    width = (width * 10) + (c - '0');

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
    int qc, c, size;
    char *ret, *lastqc = 0;

    eatspace(f);
    if ( (qc=pull(f)) != '"' && qc != '\'' )
	return 0;

    for ( ret = cursor(f); (c = pull(f)) != EOF;  ) {
	if ( c == qc )
	    lastqc = cursor(f);
	else if (c == ')' ) {
	    size = (lastqc ? lastqc : cursor(f)) - ret;
	    *sizep = size-1;
	    return ret;
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

    bzero(val, sizeof *val);

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
 * process embedded links and images
 */
static int
linkylinky(int image, MMIOT *f)
{
    int start = mmseek(f, -1);
    Footnote link;

    if ( !(linkykey(image, &link, f) && S(link.tag))  ) {
	mmseek(f, start);
	return 0;
    }

    fprintf(f->out, "<%s", image ? "img" : "a");

    fprintf(f->out, " %s=\"", image ? "src" : "href");
    puturl(T(link.link), S(link.link), f->out);
    fputc('"', f->out);

    if ( image && link.height && link.width )
	fprintf(f->out, " height=%d width=%d", link.height, link.width);

    if ( S(link.title) ) {
	fprintf(f->out, " title=\"");
	reparse(T(link.title), S(link.title), DENY_A|DENY_IMG|EXPAND_QUOTE, f);
	fputc('"', f->out);
    }
    if (image) {
	fprintf(f->out, " alt=\"");
	reparse(T(link.tag), S(link.tag), DENY_A|DENY_IMG|EXPAND_QUOTE, f);
	fputs("\">", f->out);
    }
    else {
	fputc('>', f->out);
	reparse(T(link.tag), S(link.tag), DENY_A, f);
	fprintf(f->out, "</a>");
    }
    return 1;
}


/*
 * convert an email address to a string of nonsense
 */
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

    if ( maybetag  || (size >= 3 && strncmp(cursor(f), "!--", 3) == 0) ) {
	fprintf(f->out, forbidden_tag(f) ? "&lt;" : "<");
	return 1;
    }

    if ( f->flags & DENY_A ) return 0;

    text = cursor(f);
    shift(f, size+1);

    for ( i=0; i < SZAUTOPREFIX; i++ )
	if ( strncasecmp(text, autoprefix[i], strlen(autoprefix[i])) == 0 ) {
	    fprintf(f->out, "<a href=\"");
	    puturl(text,size,f->out);
	    fprintf(f->out, "\">");
	    reparse(text, size, 0, f);
	    fprintf(f->out, "</a>");
	    return 1;
	}
    if ( maybeaddress ) {
	fprintf(f->out, "<a href=\"");
	mangle("mailto:", 7, f);
	mangle(text, size, f);
	fprintf(f->out,"\">");
	mangle(text, size, f);
	fprintf(f->out,"</a>");
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
	    fprintf(f->out, "&r%cquo;", typeofquote);
	    (*flags) &= ~bit;
	    return 1;
	}
    }
    else if ( isthisnonword(f,-1) && peek(f,1) != EOF ) {
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
					 && isthisnonword(f, 2) ) {
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
			    reparse(cursor(f)+1, j-2, 0, f);
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
		else if ( isthisspace(f,-1) && isthisspace(f,1) ) {
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
    case '3':
    case '1':	if ( isthisspace(f,-1) && peek(f,1) == '/'
				       && isthisspace(f,3) ) {
		    if ( (c == '1' && peek(f, 2) == '2')
				  || peek(f, 2) == '4' ) {
			fprintf(f->out, "&frac%c%c;", c, peek(f,2) );
			shift(f,2);
			return 1;
		    }
		}
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

	case '"':   if ( f->flags & EXPAND_QUOTE )
			fprintf(f->out, "&quot;");
		    else
			fputc(c, f->out);
		    break;
			
	case '!':   if ( (f->flags & DENY_IMG) || (peek(f,1) != '[') )
			fputc(c, f->out);
		    else {
			pull(f);
			if ( !linkylinky(1, f) ) {
			    shift(f,-1);
			    fputc(c, f->out);
			}
		    }
		    break;
	case '[':   if ( (f->flags & DENY_A) || !linkylinky(0,f) )
			fputc(c, f->out);
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
		    else if ( isthisspace(f,-1) && isthisspace(f,1) )
			fputc(c, f->out);
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
		    else
			fputc( c ? c : '\\', f->out);
		    break;

	case '<':   if ( !maybe_tag_or_link(f) )
			fprintf(f->out, "&lt;");
		    break;

	case '&':   j = (peek(f,1) == '#' ) ? 2 : 1;
		    while ( isalnum(peek(f,j)) )
			++j;

		    if ( peek(f,j) != ';' )
			fprintf(f->out, "&amp;");
		    else
			fputc(c, f->out);
		    break;

	default:    putc(c, f->out);
		    break;
	}
    }
    if ( em ) fputs("</em>", f->out);
    if ( strong ) fputs("</strong>", f->out);
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
	case '&':   fprintf(f->out, "&amp;"); break;

	case '>':   fprintf(f->out, "&gt;"); break;

	case '<':   fprintf(f->out, "&lt;"); break;

	case ' ':   if ( peek(f,1) == '`' && endofcode(escape,1, f) )
			return;
		    fputc(c, f->out);
		    break;

	case '`':   if ( endofcode(escape,0, f) )
			return;
		    fputc(c, f->out);
		    break;

	case '\\':  fputc(c, f->out);
		    if ( peek(f,1) == '>' || (c = pull(f)) == EOF )
			break;
		    fputc(c, f->out);
		    break;

	default:    fputc(c, f->out); break;
	}
    }
} /* code */


/* print a header block
 */
static void
printheader(Paragraph *pp, MMIOT *f)
{
    fprintf(f->out, "<h%d>", pp->hnumber);
    push(T(pp->text->text), S(pp->text->text), f);
    text(f);
    fprintf(f->out, "</h%d>", pp->hnumber);
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
    fputs(Begin[pp->align], f->out);
    text(f);
    fputs(End[pp->align], f->out);
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

    fprintf(f->out, "<pre><code>");
    code(0, f);
    fprintf(f->out, "</code></pre>");
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


static void
emit(Paragraph *p, char *block, MMIOT *f)
{
    if ( block )
	fprintf(f->out, "<%s>", block);

    while (( p = display(p, f) ))
	fputs("\n\n", f->out);

    if ( block )
	fprintf(f->out, "</%s>", block);
}


#if DL_TAG_EXTENSION
static void
definitionlist(Paragraph *p, MMIOT *f)
{
    Line *tag;

    if ( p ) {
	fprintf(f->out, "<dl>\n");

	for ( ; p ; p = p->next) {
	    fprintf(f->out, "<dt>");
	    if (( tag = p->text ))
		reparse(T(tag->text), S(tag->text), 0, f);
	    fprintf(f->out, "</dt>\n");

	    emit(p->down, "dd", f);
	}

	fprintf(f->out, "</dl>");
    }
}
#endif


static void
listdisplay(int typ, Paragraph *p, MMIOT* f)
{
    if ( p ) {
	fprintf(f->out, "<%cl>\n", (typ==UL)?'u':'o');

	for ( ; p ; p = p->next ) {
	    emit(p->down, "li", f);
	    putc('\n', f->out);
	}

	fprintf(f->out, "</%cl>", (typ==UL)?'u':'o');
    }
}


/* dump out a Paragraph in the desired manner
 */
static Paragraph*
display(Paragraph *p, MMIOT *f)
{
    if ( !p ) return 0;
    
    switch ( p->typ ) {
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
	fprintf(f->out, "<hr />");
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

    bzero(&f, sizeof f);
    f.out = output;
    f.flags = flags;
    
    reparse(bfr, size, 0, &f);
    return 0;
}
