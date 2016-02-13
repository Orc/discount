. tests/functions.sh

title "html comments"

rc=0
MARKDOWN_FLAGS=

try 'html comments' \
    '<!--
**hi**
-->' \
    '<!--
**hi**
-->'
    
try 'comment with trailing text' '<!-- this is -->a test' \
'<p><!-- this is -->a test</p>'

COMMENTS='<!-- 1. -->line 1

<!-- 2. -->line 2'

try 'two comments' "$COMMENTS" \
'<p><!-- 1. -->line 1</p>

<p><!-- 2. -->line 2</p>'

COMMENTS='<!-- 1. -->line 1
<!-- 2. -->line 2'

try 'two adjacent comments' "$COMMENTS" \
'<p><!-- 1. -->line 1
<!-- 2. -->line 2</p>'

try 'comment, no white space' '<!--foo-->' '<!--foo-->'

try 'comment, indented end with trailing text' \
'<!--
    -->00000' \
'<p><!--
    -->00000</p>'

try 'comment with leading text' 'Text text text<!--
    -->' \
'<p>Text text text<!--
    --></p>'

summary $0
exit $rc
