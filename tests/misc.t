. tests/functions.sh

title "misc"

rc=0
MARKDOWN_FLAGS=

try 'single paragraph' 'AAA' '<p>AAA</p>'
try '< -> &lt;' '<' '<p>&lt;</p>'
try '`>` -> <code>&gt;</code>' '`>`' '<p><code>&gt;</code></p>'
try '`` ` `` -> <code>`</code>' '`` ` ``' '<p><code>`</code></p>'
try '`` a```b ``' '`` a```b ``' '<p><code>a```b</code></p>'
try '`a^A` -> <code>a^A</code>' '`a^A`' '<p><code>a^A</code></p>'

summary $0
exit $rc
