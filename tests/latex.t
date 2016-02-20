. tests/functions.sh

# don't run these tests unless --with-latex
./markdown -V | grep LATEX >/dev/null || exit 0

title "embedded latex"

rc=0
MARKDOWN_FLAGS=

try 'latex passthrough' '\(\tex\)' '<p>\(\tex\)</p>'
try 'latex w/ special characters' 'Equation:\(a<+>b\).' \
    '<p>Equation:\(a&lt;+&gt;b\).</p>'

summary $0
exit $rc
