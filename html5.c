/* block-level tags for passing html5 blocks through the blender
 */
#include "tags.h"

void
mkd_with_html5_tags(int force_populate)
{
    static int populated = 0;

    if ( populated && !force_populate ) return;
    populated = 1;

    mkd_define_tag("ASIDE", 0);
    mkd_define_tag("FOOTER", 0);
    mkd_define_tag("HEADER", 0);
    mkd_define_tag("HGROUP", 0);
    mkd_define_tag("NAV", 0);
    mkd_define_tag("SECTION", 0);
    mkd_define_tag("ARTICLE", 0);

    mkd_sort_tags();
}
