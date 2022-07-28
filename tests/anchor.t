. tests/functions.sh


rc=0
MARKDOWN_FLAGS=

# new-style; uses a (depreciated) name=
# inside a null <a> tag

title "anchored table-of-contents support"

try '-T -ftoc,taganchor' 'anchored table of contents' \
'#H1
hi' \
'<ul>
 <li><a href="#discount-H1">H1</a></li>
</ul>
<h1 id="discount-H1"><a class="anchor" href="#discount-H1"></a>H1</h1>

<p>hi</p>'

try '-T -ftoc,taganchor' 'toc anchored item with link' \
'##[H2](H2) here' \
'<ul>
 <li>
 <ul>
  <li><a href="#discount-H2-here">H2 here</a></li>
 </ul>
 </li>
</ul>
<h2 id="discount-H2-here"><a class="anchor" href="#discount-H2-here"></a><a href="H2">H2</a> here</h2>'  

try '-T -ftoc,taganchor' 'toc anchored item with non-alpha start' \
'#1 header' \
'<ul>
 <li><a href="#discount-L1-header">1 header</a></li>
</ul>
<h1 id="discount-L1-header"><a class="anchor" href="#discount-L1-header"></a>1 header</h1>'

# Be sure to save anchor.t as UTF-8.
try '-T -ftoc,taganchor,html5anchor' 'anchored html5 multibyte chars' \
'#It’s an apostrophe' \
'<ul>
 <li><a href="#discount-It’s-an-apostrophe">It’s an apostrophe</a></li>
</ul>
<h1 id="discount-It’s-an-apostrophe"><a class="anchor" href="#discount-It’s-an-apostrophe"></a>It’s an apostrophe</h1>'

summary $0


# Check that the uniquifier works
#

title "uniquifying anchored duplicate headers"


try '-T -ftoc,taganchor' 'uniquifying anchored duplicate labels' \
'# this
# this' \
'<ul>
 <li><a href="#discount-this">this</a></li>
 <li><a href="#discount-this_0">this</a></li>
</ul>
<h1 id="discount-this"><a class="anchor" href="#discount-this"></a>this</h1>

<h1 id="discount-this_0"><a class="anchor" href="#discount-this_0"></a>this</h1>'
  

summary $0

# Check that custom anchorid works
#

title "custom anchorid headings"

try '-a test- -ftoc,taganchor' 'custom anchorid' \
'# this' \
'<h1 id="test-this"><a class="anchor" href="#test-this"></a>this</h1>'

summary $0
exit $rc
