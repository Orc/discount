. tests/functions.sh

title "crashes"

rc=0
MARKDOWN_FLAGS=

try 'zero-length input' '' ''

try 'hanging quote in list' \
' * > this should not die

no.' \
'<ul>
<li><blockquote><p>this should not die</p></blockquote></li>
</ul>


<p>no.</p>'

try 'dangling list item' ' - ' \
'<ul>
<li></li>
</ul>'

try -bHOHO 'empty []() with baseurl' '[]()' '<p><a href="HOHO"></a></p>'
try 'unclosed html block' '<table></table' '<p><table>&lt;/table</p>'
try 'unclosed style block' '<style>' '<p><style></p>'

try -ftoc 'empty header with toc' '##' '<a name="L-23-"></a>
<h1>#</h1>'

try '-x -E aloha' 'using -E to add attributes to links' '[]()' '<p><a href="" aloha></a></p>'
try '-b hello -E world' 'using both -E & -b' '[a](/b) [a](/b)' '<p><a href="hello/b" world>a</a> <a href="hello/b" world>a</a></p>'
try '-x -E aloha' 'using -E to add attributes, but with two links' '[](1.png) [](2.png)' '<p><a href="1.png" aloha></a> <a href="2.png" aloha></a></p>'

try '-T' 'excessively long ETX header prefix' '#######################################################################################################################################################################################################################################################################################################################################################################################################' \
'<ul>
 <li>
 <ul>
  <li>
  <ul>
   <li>
   <ul>
    <li>
    <ul>
     <li>
     <ul>
      <li><a href="#L-23-">#</a></li>
     </ul>
     </li>
    </ul>
    </li>
   </ul>
   </li>
  </ul>
  </li>
 </ul>
 </li>
</ul>
<a name="L-23-"></a>
<h6>#</h6>'

try '-d' 'dump an empty document' '%
%
%' ''


RESULT='<ul>
 <li><a href="#header">header</a></li>
</ul>
<a name="header"></a>
<h1>header</h1>'

./rep '#header' '\t' 400 | try -T 'a header with a bunch of trailing tabs' -heredoc "$RESULT"

./rep '#header' ' ' 400 |  try -T 'a header with a bunch of trailing spaces' -heredoc "$RESULT"

RESULT='<table>
<thead>
<tr>
<th>: Y:</th>
</tr>
</thead>
<tbody>
<tr>
<td></td>
</tr>
<tr>
<td></td>
</tr>
</tbody>
</table>'

cat << \EOF | try '-F 0x03000000' 'random input that looks like a table' -heredoc "$RESULT"
: Y:|
```|
|
```|
EOF

summary $0
exit $rc
