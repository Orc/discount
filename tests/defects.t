. tests/functions.sh

title "reported defects"

rc=0
MARKDOWN_FLAGS=

try 'masses of non-block html' \
'<span>foo</span><br>
<br>
<span>bar</span><br>' \
'<p><span>foo</span><br>
<br>
<span>bar</span><br></p>'

try -fautolink -G 'autolink + github-flavoured markdown' \
'http://foo
bar' \
'<p><a href="http://foo">http://foo</a><br/>
bar</p>'

try 'unterminated <p> block' '<p></>*' '<p><p></>*</p>'

summary $0
exit $rc
