. tests/functions.sh

title "footnotes"

rc=0
MARKDOWN_FLAGS=

try 'a line with multiple []s' '[a][] [b][]:' '<p>[a][] [b][]:</p>'
try 'a valid footnote' \
    '[alink][]

[alink]: link_me' \
    '<p><a href="link_me">alink</a></p>'

try 'a valid footnote, but encased in <>' \
    '[alink][]

[alink]: <link_me>' \
    '<p><a href="link_me">alink</a></p>'

try 'a prefixed footnote, but encased in <>' \
    '[alink][]

[alink]: <http://link.me>' \
    '<p><a href="http://link.me">alink</a></p>'

summary $0
exit $rc
