. tests/functions.sh

title "The snakepit of Markdown.pl compatibility"

rc=0
MARKDOWN_FLAGS=

try '[](single quote) text (quote)' \
    "[foo](http://Poe's law) will make this fail ('no, it won't!') here."\
    '<p><a href="http://Poe" title="s law) will make this fail ('"'no, it won't!"'">foo</a> here.</p>'

try '-f1.0' '[](unclosed <url) (MKD_1_COMPAT)' '[foo](<http://no trailing gt)' \
			'<p><a href="http://no%20trailing%20gt">foo</a></p>'

try '[](unclosed <url)' '[foo](<http://no trailing gt)' \
			'<p>[foo](&lt;http://no trailing gt)</p>'

try '<unfinished <tags> (1)' \
'<foo [bar](foo)  <s>hi</s>' \
'<p>&lt;foo <a href="foo">bar</a>  <s>hi</s></p>'
    
try '<unfinished &<tags> (2)' \
'<foo [bar](foo)  &<s>hi</s>' \
'<p>&lt;foo <a href="foo">bar</a>  &amp;<s>hi</s></p>'

try 'paragraph <br/> oddity' 'EOF  ' '<p>EOF</p>'
    
for x in tests/data/m_*.text;do
    result=`echo $x | sed -e 's/.text$/.html/'`
    try -fstrict,nopants "<tags> (`basename $x`)" "`cat $x`" "`cat $result`"
done
    
summary $0
exit $rc
