. tests/functions.sh

title "callbacks"

rc=0
MARKDOWN_FLAGS=

try -bZZZ 'url modification' \
'[a](/b)' \
'<p><a href="ZZZ/b">a</a></p>'

try -EYYY 'additional flags' \
'[a](/b)' \
'<p><a href="/b" YYY>a</a></p>'

summary $0
exit $rc
