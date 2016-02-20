. tests/functions.sh

title "math blocks"

rc=0
MARKDOWN_FLAGS=

try 'tex' '\(\tex\)' '<p>\(\tex\)</p>'
try 'tex' 'Equation:\(a+b\).' '<p>Equation:\(a+b\).</p>'
try 'tex' 'Equation:\(a>b\).' '<p>Equation:\(a&gt;b\).</p>'
try 'tex' 'Equation:\(a<b\).' '<p>Equation:\(a&lt;b\).</p>'
try 'tex' 'Equation:\(a&b\).' '<p>Equation:\(a&amp;b\).</p>'

summary $0
exit $rc
