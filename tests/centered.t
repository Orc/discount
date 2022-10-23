. tests/functions.sh

title "block centering"

rc=0
MARKDOWN_FLAGS=

try 'centered single-line paragraph' \
'->center<-' \
'<div style="text-align:center;">center</div>'

summary $0
exit $rc
