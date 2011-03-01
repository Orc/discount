. tests/functions.sh

title "tables"

rc=0
MARKDOWN_FLAGS=

try 'single-column table' \
    '|hello
|-----
|sailor' \
    '<table>
<thead>
<tr>
<th></th>
<th>hello</th>
</tr>
</thead>
<tbody>
<tr>
<td></td>
<td>sailor</td>
</tr>
</tbody>
</table>'


try 'two-column table' \
    '
  a  |  b
-----|------
hello|sailor' \
    '<table>
<thead>
<tr>
<th>  a  </th>
<th>  b</th>
</tr>
</thead>
<tbody>
<tr>
<td>hello</td>
<td>sailor</td>
</tr>
</tbody>
</table>'

try 'three-column table' \
'a|b|c
-|-|-
hello||sailor'\
    '<table>
<thead>
<tr>
<th>a</th>
<th>b</th>
<th>c</th>
</tr>
</thead>
<tbody>
<tr>
<td>hello</td>
<td></td>
<td>sailor</td>
</tr>
</tbody>
</table>'

try 'two-column table with empty cells' \
    '
  a  |  b
-----|------
hello|
     |sailor' \
    '<table>
<thead>
<tr>
<th>  a  </th>
<th>  b</th>
</tr>
</thead>
<tbody>
<tr>
<td>hello</td>
<td></td>
</tr>
<tr>
<td>     </td>
<td>sailor</td>
</tr>
</tbody>
</table>'

try 'two-column table with alignment' \
    '
  a  |  b
----:|:-----
hello|sailor' \
    '<table>
<thead>
<tr>
<th align="right">  a  </th>
<th align="left">  b</th>
</tr>
</thead>
<tbody>
<tr>
<td align="right">hello</td>
<td align="left">sailor</td>
</tr>
</tbody>
</table>'
    
try 'table with extra data column' \
    '
  a  |  b
-----|------
hello|sailor|boy' \
    '<table>
<thead>
<tr>
<th>  a  </th>
<th>  b</th>
</tr>
</thead>
<tbody>
<tr>
<td>hello</td>
<td>sailor|boy</td>
</tr>
</tbody>
</table>'


try -fnotables 'tables with -fnotables' \
    'a|b
-|-
hello|sailor' \
    '<p>a|b
&ndash;|&ndash;
hello|sailor</p>'

try 'deceptive non-table text' \
    'a | b | c

text' \
    '<p>a | b | c</p>

<p>text</p>'

try 'table headers only' \
    'a|b|c
-|-|-' \
    '<p>a|b|c
&ndash;|&ndash;|&ndash;</p>'

try 'escaped title line' \
    'A\|B
--|-
C |D' \
    '<table>
<thead>
<tr>
<th>A|B</th>
</tr>
</thead>
<tbody>
<tr>
<td>C |D</td>
</tr>
</tbody>
</table>'


try 'escaped dashes line' \
    'A |B
-\|-
C |D' \
    '<p>A |B
&ndash;|&ndash;
C |D</p>'

try 'escaped content line' \
    'A |B
--|-
C\|D' \
    '<table>
<thead>
<tr>
<th>A </th>
<th>B</th>
</tr>
</thead>
<tbody>
<tr>
<td>C|D</td>
<td></td>
</tr>
</tbody>
</table>'

try 'content line w/o dashes' \
    'A |B
--|-
CD' \
    '<p>A |B
&mdash;|&ndash;
CD</p>'

summary $0
exit $rc
