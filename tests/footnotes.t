. tests/functions.sh

title "footnotes"

rc=0
export MARKDOWN_FLAGS=2097152

try 'a line with multiple []s' '[a][] [b][]:' '<p>[a][] [b][]:</p>'
try 'a valid footnote' \
    '[alink][]

[alink]: link_me' \
    '<p><a href="link_me">alink</a></p>'

try 'a valid footnote, but encased in <>' \
    '[alink][]

[alink]: <link_me>' \
    '<p><a href="link_me">alink</a></p>'

try 'a prefixed footnote, but encased in <>' \
    '[alink][]

[alink]: <http://link.me>' \
    '<p><a href="http://link.me">alink</a></p>'

try 'a footnote to an image' \
	'Footnote[^1]

[^1]: ![Alt text](/path/to/img.jpg =80%x80%){target="_blank"}' \
	'<p>Footnote<sup id="fnref:1"><a href="#fn:1" rel="footnote">1</a></sup></p>
<div class="footnotes">
<hr/>
<ol>
<li id="fn:1">
<img src="/path/to/img.jpg" height="80%" width="80%" target="_blank" alt="Alt text" /><a href="#fnref:1" rev="footnote">&#8617;</a></li>
</ol>
</div>'

summary $0
exit $rc
