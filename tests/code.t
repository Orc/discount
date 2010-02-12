. tests/functions.sh

title "code blocks"

rc=0
MARKDOWN_FLAGS=

try 'format for code block html' \
'    this is
    code' \
    '<pre><code>this is
code
</code></pre>'

try 'unclosed single backtick' '`hi there' '<p>`hi there</p>'
try 'unclosed double backtick' '``hi there' '<p>``hi there</p>'
try 'remove space around code' '`` hi there ``' '<p><code>hi there</code></p>'

summary $0
exit $rc
