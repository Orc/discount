/* block-level tags for passing html blocks through the blender
 */
#ifndef _TAGS_D
#define _TAGS_D

#include <stdio.h>

struct kw* mkd_search_tags(MMIOT*, char *, int);
void mkd_sort_tags(MMIOT *);
void mkd_define_tag(MMIOT*, char *, int);
void ___mkd_copy_extratags(MMIOT *dst, MMIOT *src);
void ___mkd_delete_extratags(MMIOT *doc);

#endif
