/* block-level tags for passing html5 blocks through the blender
 */
#include <stdio.h>
#include "markdown.h"
#include "tags.h"

void
mkd_add_html5_tags(MMIOT* doc)
{
    mkd_define_tag(doc, "ASIDE", 0);
    mkd_define_tag(doc, "FOOTER", 0);
    mkd_define_tag(doc, "HEADER", 0);
    mkd_define_tag(doc, "NAV", 0);
    mkd_define_tag(doc, "SECTION", 0);
    mkd_define_tag(doc, "ARTICLE", 0);
}
