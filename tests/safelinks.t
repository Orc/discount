. tests/functions.sh

title "safe links"

rc=0
MARKDOWN_FLAGS=

try -fsafelink 'bogus url' '[test](bad:protocol)' \
			   '<p>[test](bad:protocol)</p>'
try -fsafelink 'illegal url' '[test](b?d:protocol)' \
			     '<p><a href="b?d:protocol">test</a></p>'
try -fsafelink 'url fragment (1)' '[test](#bar)' '<p><a href="#bar">test</a></p>'
try -fsafelink 'url fragment (2)' '[test](/bar)' '<p><a href="/bar">test</a></p>'
try -fnosafelink 'bogus url (-fnosafelink)' '[test](bad:protocol)' '<p><a href="bad:protocol">test</a></p>'


summary $0
exit $rc
