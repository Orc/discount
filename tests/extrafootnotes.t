. tests/functions.sh

title "markdown extra-style footnotes"

rc=0
MARKDOWN_FLAGS=

FOOTIE='I haz a footnote[^1]
[^1]: yes?'

try -ffootnote 'footnotes (-ffootnote)' "$FOOTIE" \
'<p>I haz a footnote<sup id="fnref:1"><a href="#fn:1" rel="footnote">1</a></sup></p>
<div class="footnotes">
<hr/>
<ol>
<li id="fn:1">
yes?<a href="#fnref:1" rev="footnote">&#8617;</a></li>
</ol>
</div>'

try -ffootnote -Cfoot 'footnotes (-ffootnote -Cfoot)' "$FOOTIE" \
'<p>I haz a footnote<sup id="footref:1"><a href="#foot:1" rel="footnote">1</a></sup></p>
<div class="footnotes">
<hr/>
<ol>
<li id="foot:1">
yes?<a href="#footref:1" rev="footnote">&#8617;</a></li>
</ol>
</div>'

try -ffootnote 'footnotes(two adjacent footnotes)' 'Hello[^1][^2]

[^1]: world
[^2]: from Hamburg' '<p>Hello<sup id="fnref:1"><a href="#fn:1" rel="footnote">1</a></sup><sup id="fnref:2"><a href="#fn:2" rel="footnote">2</a></sup></p>
<div class="footnotes">
<hr/>
<ol>
<li id="fn:1">
world<a href="#fnref:1" rev="footnote">&#8617;</a></li>
<li id="fn:2">
from Hamburg<a href="#fnref:2" rev="footnote">&#8617;</a></li>
</ol>
</div>'

try -fnofootnote 'footnotes (-fnofootnote)' "$FOOTIE" \
'<p>I haz a footnote<a href="yes?">^1</a></p>'


TSRC='Alpha[^AlphaF].

Column 1                         | Column 2
---------------------------------|--------------------------
Beta[^BetaF]                     | cell

[^AlphaF]: Alpha Footnote

[^BetaF]: Beta Footnote'

TOUT='<p>Alpha<sup id="fnref:1"><a href="#fn:1" rel="footnote">1</a></sup>.</p>

<table>
<thead>
<tr>
<th>Column 1                         </th>
<th> Column 2</th>
</tr>
</thead>
<tbody>
<tr>
<td>Beta<sup id="fnref:2"><a href="#fn:2" rel="footnote">2</a></sup>                     </td>
<td> cell</td>
</tr>
</tbody>
</table>

<div class="footnotes">
<hr/>
<ol>
<li id="fn:1">
Alpha Footnote<a href="#fnref:1" rev="footnote">&#8617;</a></li>
<li id="fn:2">
Beta Footnote<a href="#fnref:2" rev="footnote">&#8617;</a></li>
</ol>
</div>'

try -ffootnote 'footnotes inside table elements' "$TSRC" "$TOUT"


TSRC='[Test test[^test]](class:test)

<span class="test">
Test2[^testtwo]
</span>

Test3[^testthree]

<span class="test">
Test4[^testfour]
</span>

[^test]: Test footnote
[^testtwo]: Test2 footnote
[^testthree]: Test3 footnote
[^testfour]: Test4 footnote'

TOUT='<p><span class="test">Test test<sup id="fnref:1"><a href="#fn:1" rel="footnote">1</a></sup></span></p>

<p><span class="test">
Test2<sup id="fnref:2"><a href="#fn:2" rel="footnote">2</a></sup>
</span></p>

<p>Test3<sup id="fnref:3"><a href="#fn:3" rel="footnote">3</a></sup></p>

<p><span class="test">
Test4<sup id="fnref:4"><a href="#fn:4" rel="footnote">4</a></sup>
</span></p>
<div class="footnotes">
<hr/>
<ol>
<li id="fn:1">
Test footnote<a href="#fnref:1" rev="footnote">&#8617;</a></li>
<li id="fn:2">
Test2 footnote<a href="#fnref:2" rev="footnote">&#8617;</a></li>
<li id="fn:3">
Test3 footnote<a href="#fnref:3" rev="footnote">&#8617;</a></li>
<li id="fn:4">
Test4 footnote<a href="#fnref:4" rev="footnote">&#8617;</a></li>
</ol>
</div>'

try -ffootnote 'footnotes inside spans' "$TSRC" "$TOUT"

for x in tests/data/f??.text;do
    result=`echo $x | sed -e 's/.text$/.html/'`
    try -ffootnote "`basename $x`" "`cat $x`" "`cat $result`"
done
    

summary $0
exit $rc
