. tests/functions.sh

title "table-of-contents support"

rc=0
MARKDOWN_FLAGS=

try '-T -ftoc' 'table of contents' \
'#H1
hi' \
'<ul>
 <li><a href="#H1">H1</a></li>
</ul>
<h1 id="H1">H1</h1>

<p>hi</p>'

try '-T -ftoc' 'toc item with link' \
'##[H2](H2) here' \
'<ul>
 <li><ul>
  <li><a href="#H2.here">H2 here</a></li>
 </ul></li>
</ul>
<h2 id="H2.here"><a href="H2">H2</a> here</h2>'  

try '-T -ftoc' 'toc item with non-alpha start' \
'#1 header' \
'<ul>
 <li><a href="#L1.header">1 header</a></li>
</ul>
<h1 id="L1.header">1 header</h1>'

summary $0
exit $rc
