. tests/functions.sh

title "github tags"

rc=0
MARKDOWN_FLAGS=

SRC='<element-name>content</element-name>'

try 'github tags disabled by default' \
"$SRC" \
'<p>&lt;element-name>content&lt;/element-name></p>'

try -fgithubtags 'github tags' \
"$SRC" \
'<p><element-name>content</element-name></p>'

try 'normal tags pass through' \
'<a>sdf</a>' \
'<p><a>sdf</a></p>'

summary $0
exit $rc
