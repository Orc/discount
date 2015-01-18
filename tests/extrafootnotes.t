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
<p>yes?<a href="#fnref:1" rev="footnote">&#8617;</a></p></li>
</ol>
</div>'

try -ffootnote -Cfoot 'footnotes (-ffootnote -Cfoot)' "$FOOTIE" \
'<p>I haz a footnote<sup id="footref:1"><a href="#foot:1" rel="footnote">1</a></sup></p>
<div class="footnotes">
<hr/>
<ol>
<li id="foot:1">
<p>yes?<a href="#footref:1" rev="footnote">&#8617;</a></p></li>
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
<p>Alpha Footnote<a href="#fnref:1" rev="footnote">&#8617;</a></p></li>
<li id="fn:2">
<p>Beta Footnote<a href="#fnref:2" rev="footnote">&#8617;</a></p></li>
</ol>
</div>'

try -ffootnote 'footnotes inside table elements' "$TSRC" "$TOUT"

summary $0
exit $rc
