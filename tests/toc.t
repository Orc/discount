. tests/functions.sh


rc=0
MARKDOWN_FLAGS=

# old-style; uses id= tag (and collides
# with #-style css)

title "(old) table-of-contents support"

try -fidanchor '-T -ftoc' 'table of contents' \
'#H1
hi' \
'<ul>
 <li><a href="#H1">H1</a></li>
</ul>
<h1 id="H1">H1</h1>

<p>hi</p>'

try -fidanchor -fnourlencodedanchor '-T -ftoc' 'toc item with link' \
'##[H2](H2) here' \
'<ul>
 <li>
 <ul>
  <li><a href="#H2.here">H2 here</a></li>
 </ul>
 </li>
</ul>
<h2 id="H2.here"><a href="H2">H2</a> here</h2>'  

try -fidanchor -fnourlencodedanchor '-T -ftoc' 'toc item with non-alpha start' \
'#1 header' \
'<ul>
 <li><a href="#L1.header">1 header</a></li>
</ul>
<h1 id="L1.header">1 header</h1>'

try -fidanchor -furlencodedanchor '-T -ftoc' 'toc item with non-alpha start (url encoded)' \
'#1 header' \
'<ul>
 <li><a href="#1%20header">1 header</a></li>
</ul>
<h1 id="1%20header">1 header</h1>'

summary $0

# new-style; uses a (depreciated) name=
# inside a null <a> tag

title "(new) table-of-contents support"

try '-T -ftoc' 'table of contents' \
'#H1
hi' \
'<ul>
 <li><a href="#H1">H1</a></li>
</ul>
<a name="H1"></a>
<h1>H1</h1>

<p>hi</p>'

try '-T -ftoc' 'toc item with link' \
'##[H2](H2) here' \
'<ul>
 <li>
 <ul>
  <li><a href="#H2.here">H2 here</a></li>
 </ul>
 </li>
</ul>
<a name="H2.here"></a>
<h2><a href="H2">H2</a> here</h2>'  

try '-T -ftoc' 'toc item with non-alpha start' \
'#1 header' \
'<ul>
 <li><a href="#L1.header">1 header</a></li>
</ul>
<a name="L1.header"></a>
<h1>1 header</h1>'

# Be sure to save toc.t as UTF-8.
try '-T -ftoc,urlencodedanchor' 'urlencoded multibyte chars' \
'#It’s an apostrophe' \
'<ul>
 <li><a href="#It%e2%80%99s%20an%20apostrophe">It’s an apostrophe</a></li>
</ul>
<a name="It%e2%80%99s%20an%20apostrophe"></a>
<h1>It’s an apostrophe</h1>'

summary $0
exit $rc
