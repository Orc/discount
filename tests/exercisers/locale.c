/*
 * url and anchor encoding must not depend on LC_CTYPE.
 *
 * markdown(1) never calls setlocale(), so the tests/ scripts always
 * run in the C locale.  A program that links libmarkdown and sets its
 * locale from the environment (perl does this at startup) used to get
 * multibyte characters in hrefs and anchors half-encoded, because
 * isalnum()/isspace() answer for bytes 0x80-0xff by locale.
 *
 * Render each case in the C locale and again in a UTF-8 locale; the
 * two outputs must be identical.  If no UTF-8 locale is installed the
 * check is skipped rather than failed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <mkdio.h>

void
say(char *what)
{
    fputs(what,stdout);
    fflush(stdout);
}


struct cases {
    char *text;
    char *flags;
} cases[] = {
    { "[x](/a\xE2\x80\x94" "b)\n", 0 },			/* U+2014 in a link */
    { "[x](/a\\\xC3\xA9" "b)\n", 0 },			/* backslash before U+00E9 */
    { "# a \xE2\x9A\xA0" " b\n", "toc" },		/* U+26A0 in a heading */
    { "# a \xE2\x9A\xA0" " b\n", "toc,urlencodedanchor" },
    { "# \xC3\xA9t\xC3\xA9\n", "toc" },			/* heading that starts with U+00E9 */
};
#define NR(x)	(sizeof x / sizeof x[0])


static char *
render(struct cases *p)
{
    char buf[80], *html, *copy;
    int size;
    mkd_flag_t *flags = mkd_flags();
    MMIOT *doc;

    if ( p->flags ) {
	strncpy(buf, p->flags, sizeof buf - 1);
	buf[sizeof buf - 1] = 0;
	mkd_set_flag_string(flags, buf);
    }
    doc = mkd_string(p->text, strlen(p->text), flags);
    mkd_compile(doc, flags);
    size = mkd_document(doc, &html);

    copy = malloc(size + 1);
    memcpy(copy, html, size);
    while ( size > 0 && copy[size-1] == '\n' )	/* for printing a mismatch */
	--size;
    copy[size] = 0;

    mkd_cleanup(doc);
    mkd_free_flags(flags);
    return copy;
}


int
main(void)
{
    static char *utf8[] = { "C.UTF-8", "en_US.UTF-8", "UTF-8", "C.utf8", "en_US.utf8" };
    char *expected[NR(cases)];
    char *got;
    int i, failed = 0;

    say("check locale-independent encoding: ");

    setlocale(LC_CTYPE, "C");
    for ( i=0; i < NR(cases); i++ )
	expected[i] = render(&cases[i]);

    for ( i=0; i < NR(utf8); i++ )
	if ( setlocale(LC_CTYPE, utf8[i]) )
	    break;

    if ( i == NR(utf8) ) {
	say("no UTF-8 locale, skipped\n");
	exit(0);
    }

    for ( i=0; i < NR(cases); i++ ) {
	got = render(&cases[i]);
	if ( strcmp(got, expected[i]) != 0 ) {
	    if ( !failed++ )
		say("FAILED\n");
	    fprintf(stdout, "  case %d, %s:\n    C:        %s\n    %-9s %s\n",
		    i, cases[i].flags ? cases[i].flags : "no flags",
		    expected[i], setlocale(LC_CTYPE, 0), got);
	}
	free(got);
    }

    if ( failed )
	exit(1);

    say("ok\n");
    exit(0);
}
