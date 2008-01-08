#include <stdio.h>
#include "markdown.h"
#include "cstring.h"

struct frame {
    int indent;
    char c;
};

static STRING(struct frame) stack;
    
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
pushpfx(int indent, char c)
{
    struct frame *q = &EXPAND(stack);

    q->indent = indent;
    q->c = c;
}


static void
poppfx()
{
    S(stack)--;
}


static void
changepfx(char c)
{
    char ch;

    if ( !S(stack) ) return;

    ch = T(stack)[S(stack)-1].c;

    if ( ch == '+' || ch == '|' )
	T(stack)[S(stack)-1].c = c;
}


static void
printpfx()
{
    int i;
    char c;

    if ( !S(stack) ) return;

    c = T(stack)[S(stack)-1].c;

    if ( c == '+' || c == '-' ) {
	printf("--%c", c);
	T(stack)[S(stack)-1].c = (c == '-') ? ' ' : '|';
    }
    else
	for ( i=0; i < S(stack); i++ ) {
	    if ( i )
		printf("  ");
	    printf("%*s%c", T(stack)[i].indent + 2, " ", T(stack)[i].c);
	    if ( T(stack)[i].c == '`' )
		T(stack)[i].c = ' ';
	}
    printf("--");
}


static void
dumptree(Paragraph *pp, void *context)
{
    int count;
    Line *p;
    int d;
    static char *Begin[] = { 0, "P", "center" };

    while ( pp ) {
	if ( !pp->next )
	    changepfx('`');
	printpfx();

	d = printf("[%s", Pptype(pp->typ));
	if ( pp->align )
	    d += printf(", <%s>", Begin[pp->align]);

	for (count=0, p=pp->text; p; ++count, (p = p->next) )
	    ;

	if ( count )
	    d += printf(", %d line%s", count, (count==1)?"":"s");

	d += printf("]");

	if ( pp->down ) {
	    pushpfx(d, pp->down->next ? '+' : '-');
	    dumptree(pp->down, context);
	    poppfx();
	}
	else putchar('\n');
	pp = pp->next;
    }
}


int
mkd_dump(Line *input, FILE *output, int flags)
{
    typedef void (*emitter)(Paragraph*,void*);
    extern int __mkd_internal_markdown(Line*, FILE*, int, emitter);

    CREATE(stack);
    return __mkd_internal_markdown(input,output,flags,dumptree);
    DELETE(stack);
}
