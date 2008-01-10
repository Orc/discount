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


/* skip past space characters
 */
static void
skipblankc(MMIOT *f)
{
    while ( isspace(peek(f,1)) )
	pull(f);
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
#if 0
	case '&':   if ( (size < 5) || strncmp(s, "&amp;", 5) != 0 ) {
			fputs("&amp;", f);
			break;
		    }
		    /*else fall into... */
#endif
	default:    if ( c == '"' || c == ' ' || !isprint(c) )
			fprintf(f, "%%%02x", c);
		    else
			fputc(c, f);
		    break;
	}
    }
}



static int
broketmatch(MMIOT *f, int i)
{
    int c = peek(f, 1+i);

    return (c == '>') || (c == EOF);
}


static int
spacematch(MMIOT *f, int i)
{
    int c = peek(f, i);

    return (c == EOF) || (c == ')') || isspace(c);
}


/*
 * process embedded links and images
 */
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

    switch (c) {
	char *tag;
	Footnote key, *ret;
	int (*matched)(MMIOT*,int);
    
    case '(':
	matched = (peek(f,1) == '<') ? broketmatch : spacematch;

	for ( size = 0; !(*matched)(f, size+1); ++size)
	    ;

	fprintf(f->out, "<%s %s=\"", image ? "img" : "a",
				     image ? "src" : "href");
	if ( peek(f,1) == '<' ) {
	    puturl(cursor(f)+1, size, f->out);
	    shift(f,3);	/* shift past '(', '<' && '>' */
	}
	else
	    puturl(cursor(f), size, f->out);
	fputc('"', f->out);
	
	shift(f, size);
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
	    /* title;  now run up to the end of the () block, then
	     * back down to the last quote character we found.
	     * This allows evil things like "this "is" a "title"")
	     */
	    for ( size=0; ((c=peek(f,size+2)) != EOF) && (c != ')') ; ++size )
		;
	    while ( size && (peek(f,size+2) != qc) )
		--size;

	    fprintf(f->out, " title=\"");
	    reparse(cursor(f)+1, size, DENY_A|DENY_IMG|EXPAND_QUOTE, f);
	    fputc('"', f->out);

	    shift(f, size+2);
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
	               sizeof key, (stfu)__mkd_footsort);

	fprintf(f->out, image ?  "<img" : "<a");

	if ( ret ) {
	    if ( S(ret->link) ) {
		fprintf(f->out, " %s=\"", image ? "src" : "href");
		puturl(T(ret->link), S(ret->link)-1, f->out);
		fputc('"', f->out);
	    }
	    if ( S(ret->title) ) {
		fprintf(f->out, " title=\"");
		reparse(T(ret->title), S(ret->title)-1, DENY_A|DENY_IMG|EXPAND_QUOTE, f);
		fputc('"', f->out);
	    }

	    if ( image && ret->height && ret->width )
		fprintf(f->out, " height=%d width=%d",
				ret->height, ret->width);
	}
	break;
    }

    fprintf(f->out, image  ? " alt=\"" : ">");
    reparse(label, labelsize-2, image ? DENY_A|DENY_IMG|EXPAND_QUOTE : 0, f);
    fprintf(f->out, image ? "\">" : "</a>");
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

	case '"':   if ( f->flags & EXPAND_QUOTE )
			fprintf(f->out, "&quot;", c);
		    else
			fputc(c, f->out);
		    break;
			
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
		    else if ( isspace(peek(f,-1)) && isspace(peek(f,1)) )
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
#if 0
		    else if ( c == '>' )
			fprintf(f->out, "&gt;");
#endif
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

	case '>':   fprintf(f->out, "&gt;"); break;

	case '<':   fprintf(f->out, "&lt;"); break;

	case '`':   switch (escape) {
		    case 2: if ( peek(f,1) == '`' ) {
				shift(f,1);
		    case 1:     return;
		            }
		    }
		    fputc(c, f->out); break;

	default:    fputc(c, f->out); break;

	case '\\':  fputc(c, f->out);
		    if ( peek(f,1) == '>' || (c = pull(f)) == EOF )
			break;
		    fputc(c, f->out);
		    break;
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
    static char *Begin[] = { "", "<p>",   "<center>"   };
    static char *End[]   = { "", "</p>","</center>"};

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


/* external interface to emit() for markdown.c to use
 */
void
__mkd_generatehtml(Paragraph *p, MMIOT *f)
{
    emit(p, 0, f);
    putc('\n', f->out);
}


/* external interface to reparse() for markdown.c to use
 */
void
__mkd_reparse(char *bfr, int size, int flags, MMIOT *f)
{
    reparse(bfr,size,flags,f);
}
