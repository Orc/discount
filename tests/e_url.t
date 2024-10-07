. tests/functions.sh

title "mkd_basename (e_url)"

rc=0
MARKDOWN_FLAGS=

try -bHOHO 'empty []() with baseurl' '[]()' '<p><a href="HOHO"></a></p>'
try -bHOHO '[]() with full url' '[](http://foo)' '<p><a href="http://foo"></a></p>'
try -bHOHO '[]() with url fragment' '[](/foo)'   '<p><a href="HOHO/foo"></a></p>'

summary $0
exit $rc
