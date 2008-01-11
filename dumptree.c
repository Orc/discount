/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include "markdown.h"
#include "cstring.h"

struct frame {
    int indent;
    char c;
};

typedef STRING(struct frame) Stack;

static char *
Pptype(int typ)
{
    switch (typ) {
    case WHITESPACE: return "whitespace";
    case CODE      : return "code";
    case QUOTE     : return "quote";
    case MARKUP    : return "markup";
    case HTML      : return "html";
    case DL        : return "dl";
    case UL        : return "ul";
    case OL        : return "ol";
    case LISTITEM  : return "item";
    case HDR       : return "header";
    case HR        : return "HR";
    default        : return "mystery node!";
    }
}

static void
pushpfx(int indent, char c, Stack *sp)
{
    struct frame *q = &EXPAND(*sp);

    q->indent = indent;
    q->c = c;
}


static void
poppfx(Stack *sp)
{
    S(*sp)--;
}


static void
changepfx(Stack *sp, char c)
{
    char ch;

    if ( !S(*sp) ) return;

    ch = T(*sp)[S(*sp)-1].c;

    if ( ch == '+' || ch == '|' )
	T(*sp)[S(*sp)-1].c = c;
}


static void
printpfx(Stack *sp)
{
    int i;
    char c;

    if ( !S(*sp) ) return;

    c = T(*sp)[S(*sp)-1].c;

    if ( c == '+' || c == '-' ) {
	printf("--%c", c);
	T(*sp)[S(*sp)-1].c = (c == '-') ? ' ' : '|';
    }
    else
	for ( i=0; i < S(*sp); i++ ) {
	    if ( i )
		printf("  ");
	    printf("%*s%c", T(*sp)[i].indent + 2, " ", T(*sp)[i].c);
	    if ( T(*sp)[i].c == '`' )
		T(*sp)[i].c = ' ';
	}
    printf("--");
}


static void
dumptree(Paragraph *pp, Stack *sp)
{
    int count;
    Line *p;
    int d;
    static char *Begin[] = { 0, "P", "center" };

    while ( pp ) {
	if ( !pp->next )
	    changepfx(sp, '`');
	printpfx(sp);

	d = printf("[%s", Pptype(pp->typ));
	if ( pp->align )
	    d += printf(", <%s>", Begin[pp->align]);

	for (count=0, p=pp->text; p; ++count, (p = p->next) )
	    ;

	if ( count )
	    d += printf(", %d line%s", count, (count==1)?"":"s");

	d += printf("]");

	if ( pp->down ) {
	    pushpfx(d, pp->down->next ? '+' : '-', sp);
	    dumptree(pp->down, sp);
	    poppfx(sp);
	}
	else putchar('\n');
	pp = pp->next;
    }
}


int
mkd_dump(Document *doc, FILE *output, int flags, char *title)
{
    Stack stack;

    if (mkd_compile(doc, flags) ) {

	CREATE(stack);
	pushpfx(printf("%s", title), doc->code->next ? '+' : '-', &stack);
	dumptree(doc->code, &stack);
	DELETE(stack);

	mkd_cleanup(doc);
	return 0;
    }
    return -1;
}
