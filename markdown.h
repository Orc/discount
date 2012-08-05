#ifndef _MARKDOWN_D
#define _MARKDOWN_D

#include "cstring.h"

/* reference-style links (and images) are stored in an array
 * of footnotes.
 */
typedef struct footnote {
    Cstring tag;		/* the tag for the reference link */
    Cstring link;		/* what this footnote points to */
    Cstring title;		/* what it's called (TITLE= attribute) */
    Cstring args;               /* stuff after the title */
    int height, width;		/* dimensions (for image link) */
    int dealloc;		/* deallocation needed? */
    int refnumber;
    int flags;
#define EXTRA_BOOKMARK	0x01
#define REFERENCED	0x02
} Footnote;

/* each input line is read into a Line, which contains the line,
 * the offset of the first non-space character [this assumes 
 * that all tabs will be expanded to spaces!], and a pointer to
 * the next line.
 */
typedef struct line {
    Cstring text;
    struct line *next;
    int dle;			/* leading indent on the line */
    int flags;			/* special attributes for this line */
#define PIPECHAR	0x01		/* line contains a | */
#define CHECKED		0x02

    enum { chk_text, chk_code,
	   chk_hr, chk_dash,
	   chk_tilde, chk_equal } kind;
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
    char *ident;		/* %id% tag for QUOTE */
    enum { WHITESPACE=0, CODE, QUOTE, MARKUP,
	   HTML, STYLE, DL, UL, OL, AL, LISTITEM,
	   HDR, HR, TABLE, CAPTION, FIGURE, SOURCE } typ;
    enum { IMPLICIT=0, PARA, CENTER} align;
    int hnumber;		/* <Hn> for typ == HDR */
} Paragraph;

enum { ETX, SETEXT };	/* header types */


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
    mkd_free_t e_free;		/* edit/flags callback memory deallocator */
} Callback_data;


struct escaped { 
    char *text;
    struct escaped *up;
} ;


/* a magic markdown io thing holds all the data structures needed to
 * do the backend processing of a markdown document
 */
typedef struct mmiot {
    Cstring out;
    Cstring in;
    Qblock Q;
    int isp;
    int reference;
    struct escaped *esc;
    char *ref_prefix;
    STRING(Footnote) *footnotes;
    DWORD flags;
#define MKD_NOLINKS	0x00000001
#define MKD_NOIMAGE	0x00000002
#define MKD_NOPANTS	0x00000004
#define MKD_NOHTML	0x00000008
#define MKD_STRICT	0x00000010
#define MKD_TAGTEXT	0x00000020
#define MKD_NO_EXT	0x00000040
#define MKD_CDATA	0x00000080
#define MKD_NOSUPERSCRIPT 0x00000100
#define MKD_NORELAXED	0x00000200
#define MKD_NOTABLES	0x00000400
#define MKD_NOSTRIKETHROUGH 0x00000800
#define MKD_TOC		0x00001000
#define MKD_1_COMPAT	0x00002000
#define MKD_AUTOLINK	0x00004000
#define MKD_SAFELINK	0x00008000
#define MKD_NOHEADER	0x00010000
#define MKD_TABSTOP	0x00020000
#define MKD_NODIVQUOTE	0x00040000
#define MKD_NOALPHALIST	0x00080000
#define MKD_NODLIST	0x00100000
#define MKD_EXTRA_FOOTNOTE 0x00200000
#define MKD_NOFIGURES   0x00400000
#define MKD_NOHDRLINKS  0x00800000
#define IS_LABEL	0x08000000
#define USER_FLAGS	0x0FFFFFFF
#define INPUT_MASK	(MKD_NOHEADER|MKD_TABSTOP)

    Callback_data *cb;
} MMIOT;


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


extern int  mkd_firstnonblank(Line *);
extern int  mkd_compile(Document *, DWORD);
extern int  mkd_document(Document *, char **);
extern int  mkd_generatehtml(Document *, FILE *);
extern int  mkd_css(Document *, char **);
extern int  mkd_generatecss(Document *, FILE *);
#define mkd_style mkd_generatecss
extern int  mkd_xml(char *, int , char **);
extern int  mkd_generatexml(char *, int, FILE *);
extern void mkd_cleanup(Document *);
extern int  mkd_line(char *, int, char **, DWORD);
extern int  mkd_generateline(char *, int, FILE*, DWORD);
#define mkd_text mkd_generateline
extern void mkd_basename(Document*, char *);

typedef int (*mkd_sta_function_t)(const int,const void*);
extern void mkd_string_to_anchor(char*,int, mkd_sta_function_t, void*, int);

extern Document *mkd_in(FILE *, DWORD);
extern Document *mkd_string(const char*,int, DWORD);

extern Document *gfm_in(FILE *, DWORD);
extern Document *gfm_string(const char*,int, DWORD);

extern void mkd_initialize();
extern void mkd_shlib_destructor();

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
extern void ___mkd_reparse(char *, int, int, MMIOT*, char*);
extern void ___mkd_emblock(MMIOT*);
extern void ___mkd_tidy(Cstring *);

extern Document *__mkd_new_Document();
extern void __mkd_enqueue(Document*, Cstring *);
extern void __mkd_header_dle(Line *);

extern int  __mkd_io_strget(struct string_stream *);

#endif/*_MARKDOWN_D*/
