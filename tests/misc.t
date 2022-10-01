. tests/functions.sh

title "misc"

rc=0
MARKDOWN_FLAGS=

try 'single paragraph' 'AAA' '<p>AAA</p>'
try '< -> &lt;' '<' '<p>&lt;</p>'
try -f1.0 '< do not sanitize in markdown 1.0 mode' '<' '<p><</p>'

summary $0
exit $rc
