#ifndef _MARKDOWN_D
#define _MARKDOWN_D

#include "config.h"
#include "cstring.h"

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#elif HAVE_STDINT_H
#   include <stdint.h>
#endif

/* flags, captured into a named type
 */
enum {  MKD_NOLINKS=0,		/* don't do link processing, block <a> tags  */
	MKD_NOIMAGE,		/* don't do image processing, block <img> */
	MKD_NOPANTS,		/* don't run smartypants() */
	MKD_NOHTML,		/* don't allow raw html through AT ALL */
	MKD_NORMAL_LISTITEM,	/* disable github-style checkbox lists */
	MKD_TAGTEXT,		/* process text inside an html tag */
	MKD_NO_EXT,		/* don't allow pseudo-protocols */
#define MKD_NOEXT MKD_NO_EXT
	MKD_EXPLICITLIST,	/* don't combine numbered/bulletted lists */
	MKD_CDATA,		/* generate code for xml ![CDATA[...]] */
	MKD_NOSUPERSCRIPT,	/* no A^B */
	MKD_STRICT,		/* conform to Markdown standard as implemented in Markdown.pl */
	MKD_NOTABLES,		/* disallow tables */
	MKD_NOSTRIKETHROUGH,	/* forbid ~~strikethrough~~ */
	MKD_1_COMPAT,		/* compatibility with MarkdownTest_1.0 */
	MKD_TOC,		/* do table-of-contents processing */
	MKD_AUTOLINK,		/* make http://foo.com link even without <>s */
	MKD_NOHEADER,		/* don't process header blocks */
	MKD_TABSTOP,		/* expand tabs to 4 spaces */
	MKD_SAFELINK,		/* paranoid check for link protocol */
	MKD_NODIVQUOTE,		/* forbid >%class% blocks */
	MKD_NOALPHALIST,	/* forbid alphabetic lists */
	MKD_EXTRA_FOOTNOTE,	/* enable markdown extra-style footnotes */
	MKD_NOSTYLE,		/* don't extract <style> blocks */
	MKD_DLDISCOUNT,		/* enable discount-style definition lists */
	MKD_DLEXTRA,		/* enable extra-style definition lists */
	MKD_FENCEDCODE,		/* enabled fenced code blocks */
	MKD_IDANCHOR,		/* use id= anchors for TOC links */
	MKD_GITHUBTAGS,		/* allow dash and underscore in element names */
	MKD_URLENCODEDANCHOR,	/* urlencode non-identifier chars instead of replacing with dots */
	MKD_LATEX,		/* handle embedded LaTeX escapes */
			/* end of user flags */
	IS_LABEL,
	MKD_NR_FLAGS };


typedef struct { char bit[MKD_NR_FLAGS]; } mkd_flag_t;

void mkd_init_flags(mkd_flag_t *p);

#define is_flag_set(flags, item)	((flags)->bit[item])
#define set_mkd_flag(flags, item)	((flags)->bit[item] = 1)
#define clear_mkd_flag(flags, item)	((flags)->bit[item] = 0)

#define COPY_FLAGS(dst,src)	memcpy(&dst,&src,sizeof dst)

void ___mkd_or_flags(mkd_flag_t* dst, mkd_flag_t* src);
int ___mkd_different(mkd_flag_t* dst, mkd_flag_t* src);
int ___mkd_any_flags(mkd_flag_t* dst, mkd_flag_t* src);

#define ADD_FLAGS(dst,src)	___mkd_or_flags(dst,src)
#define DIFFERENT(dst,src)	___mkd_different(dst,src)
#define ANY_FLAGS(dst,src)	___mkd_any_flags(dst,src)

/* each input line is read into a Line, which contains the line,
 * the offset of the first non-space character [this assumes 
 * that all tabs will be expanded to spaces!], and a pointer to
 * the next line.
 */
typedef enum { chk_text, chk_code,
	       chk_hr, chk_dash,
	       chk_tilde, chk_backtick,
	       chk_equal } line_type;
typedef struct line {
    Cstring text;
    struct line *next;
    int dle;			/* leading indent on the line */
    int has_pipechar;		/* line contains a | */
    int is_checked;
    line_type kind;
    int is_fenced;		/* line inside a fenced code block (ick) */
    char *fence_class;		/* fenced code class (ick) */
    int count;
} Line;


/* a paragraph is a collection of Lines, with links to the next paragraph
 * and (if it's a QUOTE, UL, or OL) to the reparsed contents of this
 * paragraph.
 */
typedef struct paragraph {
    struct paragraph *next;	/* next paragraph */
    struct paragraph *down;	/* recompiled contents of this paragraph */
    struct line *text;		/* all the text in this paragraph */
    char *label;		/* toc label, uniqued */
    char *ident;		/* %id% tag for QUOTE */
    char *lang;			/* lang attribute for CODE */
    enum { WHITESPACE=0, CODE, QUOTE, MARKUP,
	   HTML, STYLE, DL, UL, OL, AL, LISTITEM,
	   HDR, HR, TABLE, SOURCE } typ;
    enum { IMPLICIT=0, PARA, CENTER} align;
    int hnumber;		/* <Hn> for typ == HDR */
    int para_flags;
#define GITHUB_CHECK		0x01
#define IS_CHECKED		0x02
} Paragraph;

typedef ANCHOR(Paragraph) ParagraphRoot;

enum { ETX, SETEXT };	/* header types */

/* reference-style links (and images) are stored in an array
 * of footnotes.
 */
typedef struct footnote {
    Cstring tag;		/* the tag for the reference link */
    Cstring link;		/* what this footnote points to */
    Cstring title;		/* what it's called (TITLE= attribute) */
    Paragraph *text;		/* EXTRA_FOOTNOTE content */
    
    int height, width;		/* dimensions (for image link) */
    int dealloc;		/* deallocation needed? */
    int refnumber;
    int fn_flags;
#define EXTRA_FOOTNOTE	0x01
#define REFERENCED	0x02
} Footnote;


typedef struct block {
    enum { bTEXT, bSTAR, bUNDER } b_type;
    int  b_count;
    char b_char;
    Cstring b_text;
    Cstring b_post;
} block;

typedef STRING(block) Qblock;


typedef char* (*mkd_callback_t)(const char*, const int, void*);
typedef void  (*mkd_free_t)(char*, void*);

typedef struct callback_data {
    void *e_data;		/* private data for callbacks */
    mkd_callback_t e_url;	/* url edit callback */
    mkd_callback_t e_flags;	/* extra href flags callback */
    mkd_callback_t e_anchor;	/* callback for anchor types */
    mkd_free_t e_free;		/* edit/flags callback memory deallocator */
    mkd_callback_t e_codefmt;	/* codeblock formatter (for highlighting) */
} Callback_data;


struct escaped { 
    char *text;
    struct escaped *up;
} ;


struct footnote_list {
    int reference;
    STRING(Footnote) note;
} ;


/* a magic markdown io thing holds all the data structures needed to
 * do the backend processing of a markdown document
 */
typedef struct mmiot {
    Cstring out;
    Cstring in;
    Qblock Q;
    char last;	/* last text character added to out */
    int isp;
    struct escaped *esc;
    char *ref_prefix;
    struct footnote_list *footnotes;
    mkd_flag_t flags;

    Callback_data *cb;
} MMIOT;


#define MKD_EOLN	'\r'


/*
 * the mkdio text input functions return a document structure,
 * which contains a header (retrieved from the document if
 * markdown was configured * with the * --enable-pandoc-header
 * and the document begins with a pandoc-style header) and the
 * root of the linked list of Lines.
 */
typedef struct document {
    int magic;			/* "I AM VALID" magic number */
#define VALID_DOCUMENT		0x19600731
    Line *title;
    Line *author;
    Line *date;
    ANCHOR(Line) content;	/* uncompiled text, not valid after compile() */
    Paragraph *code;		/* intermediate code generated by compile() */
    int compiled;		/* set after mkd_compile() */
    int dirty;			/* flags or callbacks changed */
    int html;			/* set after (internal) htmlify() */
    int tabstop;		/* for properly expanding tabs (ick) */
    char *ref_prefix;
    MMIOT *ctx;			/* backend buffers, flags, and structures */
    Callback_data cb;		/* callback functions & private data */
} Document;


/*
 * economy FILE-type structure for pulling characters out of a
 * fixed-length string.
 */
struct string_stream {
    const char *data;	/* the unread data */
    int   size;		/* and how much is there? */
} ;

/*
 * sneakily back-define the published interface (leaving the old functions for v2 compatibility)
 */

#define mkd_in mkd3_in
#define mkd_string mkd3_string
#define gfm_in gfm3_in
#define gfm_string gfm3_string
#define mkd_compile mkd3_compile
#define mkd_dump mkd3_dump
#define markdown markdown3
#define mkd_line mkd3_line
#define mkd_xhtmlpage mkd3_xhtmlpage
#define mkd_generateline mkd3_generateline
#define mkd_flags_are mkd3_flags_are


/* the published interface (plus a few local functions that I need to fix)
 */
extern int  mkd_firstnonblank(Line *);
extern int  mkd_compile(Document *, mkd_flag_t*);
extern int  mkd_document(Document *, char **);
extern int  mkd_generatehtml(Document *, FILE *);
extern int  mkd_css(Document *, char **);
extern int  mkd_generatecss(Document *, FILE *);
#define mkd_style mkd_generatecss
extern int  mkd_xml(char *, int , char **);
extern int  mkd_generatexml(char *, int, FILE *);
extern void mkd_cleanup(Document *);
extern int  mkd_line(char *, int, char **, mkd_flag_t*);
extern int  mkd_generateline(char *, int, FILE*, mkd_flag_t*);
#define mkd_text mkd_generateline
extern void mkd_basename(Document*, char *);

typedef int (*mkd_sta_function_t)(const int,const void*);
extern void mkd_string_to_anchor(char*,int, mkd_sta_function_t, void*, int, MMIOT *);

extern Document *mkd_in(FILE *, mkd_flag_t*);
extern Document *mkd_string(const char*, int, mkd_flag_t*);

extern Document *gfm_in(FILE *, mkd_flag_t*);
extern Document *gfm_string(const char*,int, mkd_flag_t*);

extern void mkd_initialize(void);
extern void mkd_shlib_destructor(void);

extern void mkd_ref_prefix(Document*, char*);

/* internal resource handling functions.
 */
extern void ___mkd_freeLine(Line *);
extern void ___mkd_freeLines(Line *);
extern void ___mkd_freeParagraph(Paragraph *);
extern void ___mkd_freefootnote(Footnote *);
extern void ___mkd_freefootnotes(MMIOT *);
extern void ___mkd_initmmiot(MMIOT *, void *);
extern void ___mkd_freemmiot(MMIOT *, void *);
extern void ___mkd_freeLineRange(Line *, Line *);
extern void ___mkd_xml(char *, int, FILE *);
extern void ___mkd_reparse(char *, int, mkd_flag_t*, MMIOT*, char*);
extern void ___mkd_emblock(MMIOT*);
extern void ___mkd_tidy(Cstring *);

extern Document *__mkd_new_Document(void);
extern void __mkd_enqueue(Document*, Cstring *);
extern void __mkd_trim_line(Line *, int);

extern int  __mkd_io_strget(struct string_stream *);

/* toc uniquifier
 */
extern void ___mkd_uniquify(ParagraphRoot *, Paragraph *);
    
/* utility function to do some operation and exit the current function
 * if it fails
 */
#define DO_OR_DIE(op) if ( (op) == EOF ) return EOF; else 1

#endif/*_MARKDOWN_D*/
