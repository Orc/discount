/* block-level tags for passing html blocks through the blender
 */
#include "config.h"

#define __WITHOUT_AMALLOC 1
#include <stdio.h>
#include "markdown.h"
#include "cstring.h"
#include "tags.h"

/* the standard collection of tags are built and sorted when
 * discount is configured, so all we need to do is pull them
 * in and use them.
 *
 * Additional tags still need to be allocated, sorted, and deallocated.
 */
#include "blocktags"


/* define an additional html block tag
 */
void
mkd_define_tag(MMIOT *doc, char *id, int selfclose)
{
    struct kw *p;

    /* only add the new tag if it doesn't exist in
     * either the standard or extra tag tables.
     */
    if ( !(p = mkd_search_tags(doc, id, strlen(id))) ) {
	p = &EXPAND(doc->extratags);
	p->id = strdup(id);
	p->size = strlen(id);
	p->selfclose = selfclose;

	mkd_sort_tags(doc);
    }
}


/* case insensitive string sort (for qsort() and bsearch() of block tags)
 */
static int
casort(struct kw *a, struct kw *b)
{
    if ( a->size != b->size )
	return a->size - b->size;
    return strncasecmp(a->id, b->id, b->size);
}


/* stupid cast to make gcc shut up about the function types being
 * passed into qsort() and bsearch()
 */
typedef int (*stfu)(const void*,const void*);


/* sort the list of extra html block tags for later searching
 */
void
mkd_sort_tags(MMIOT *doc)
{
    if ( S(doc->extratags) )
	qsort(T(doc->extratags), S(doc->extratags), sizeof(struct kw), (stfu)casort);
}


/* look for a token in the html block tag list
 */
struct kw*
mkd_search_tags(MMIOT *doc, char *pat, int len)
{
    struct kw key;
    struct kw *ret;
    
    key.id = pat;
    key.size = len;
    
    if ( (ret=bsearch(&key,blocktags,NR_blocktags,sizeof key,(stfu)casort)) )
	return ret;

    if ( S(doc->extratags) )
	return bsearch(&key,T(doc->extratags),S(doc->extratags),sizeof key,(stfu)casort);
    
    return 0;
}


/* delete an extratags structure
 */

void
___mkd_delete_extratags(MMIOT *doc)
{
    int i;

    for ( i=0; i<S(doc->extratags); i++ )
	free(T(doc->extratags)[i].id);

    S(doc->extratags) = 0;
}


/* duplicate an extratags structure
 */
void
___mkd_copy_extratags(MMIOT *dst, MMIOT *src)
{
    int i;

    if ( (src == NULL) || (dst == NULL) )
	return;

    if ( S(dst->extratags) )
	___mkd_delete_extratags(dst);

    for (i=0; i< S(src->extratags); i++ ) {
	EXPAND(dst->extratags);
	T(dst->extratags)[i].id = strdup(T(src->extratags)[i].id);
	T(dst->extratags)[i].size = T(src->extratags)[i].size;
	T(dst->extratags)[i].selfclose = T(src->extratags)[i].selfclose;
    }
}
