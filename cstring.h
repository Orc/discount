/* two template types:  STRING(t) which defines a pascal-style string
 * of element (t) [STRING(char) is the closest to the pascal string],
 * and ANCHOR(t) which defines a baseplate that a linked list can be
 * built up from.   [The linked list /must/ contain a ->next pointer
 * for linking the list together with.]
 */
#ifndef _CSTRING_D
#define _CSTRING_D

#include <stdlib.h>

/* expandable Pascal-style string.
 */
#define STRING(type)	struct { type *text; int size, alloc; }

#define RESERVE(x,c)	(x).text = malloc(sizeof T(x)[0] * (((x).size=0),((x).alloc=(c))) )
#define CREATE(x)	RESERVE(x,100)
#define EXPAND(x)	(x).text[((x).size < (x).alloc \
			    ? 0 \
			    : !((x).text = realloc((x).text, sizeof T(x)[0] * ((x).alloc += 100)))), \
			(x).size++]

#define DELETE(x)	(x).alloc ? (free(T(x)), S(x) x.alloc = 0) \
				  : ( S(x) = 0 )
#define CLIP(t,i,sz)	\
	    ( ((i) >= 0) && ((sz) > 0) && (((i)+(sz)) <= S(t)) ) ? \
	    (memmove(&T(t)[i], &T(t)[i+sz], (S(t)-(i+sz)+1)*sizeof(T(t)[0])), \
		S(t) -= (sz)) : -1

/* reference-style links (and images) are stored in an array
 */
#define T(x)		(x).text
#define S(x)		(x).size

/* abstract anchor type that defines a list base
 * with a function that attaches an element to
 * the end of the list.
 *
 * the list base field is named .text so that the T()
 * macro will work with it.
 */
#define ANCHOR(t)	struct { t *text, *end; }

#define ATTACH(t, p)	( (t).text ?( ((t).end->next = (p)), ((t).end = (p)) ) \
				   :( ((t).text = (t).end = (p)) ) )

typedef STRING(char) Cstring;

#endif/*_CSTRING_D*/
