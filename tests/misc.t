. tests/functions.sh

title "misc"

rc=0
MARKDOWN_FLAGS=

try 'single paragraph' 'AAA' '<p>AAA</p>'
try '< -> &lt;' '<' '<p>&lt;</p>'
try '`>` -> <code>&gt;</code>' '`>`' '<p><code>&gt;</code></p>'
try '`` ` `` -> <code>`</code>' '`` ` ``' '<p><code>`</code></p>'

summary $0
exit $rc
