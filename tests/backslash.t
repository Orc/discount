. tests/functions.sh

title "backslash escapes"

rc=0
MARKDOWN_FLAGS=

try 'backslashes in []()' '[foo](http://\this\is\.a\test\(here\))' \
'<p><a href="http://\this\is.a\test(here)">foo</a></p>'

try -fautolink 'autolink url with trailing \' \
    'http://a.com/\' \
    '<p><a href="http://a.com/\">http://a.com/\</a></p>'


try 'backslashes before <text' '\<code>' '<p>\<code></p>'
try 'backslashes before <{EOF}' '\<' '<p>&lt;</p>'
try 'backslashes before <[space]' '\< j' '<p>&lt; j</p>'

summary $0
exit $rc
