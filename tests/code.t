. tests/functions.sh

title "code blocks"

rc=0
MARKDOWN_FLAGS=

try 'format for code block html' \
'    this is
    code' \
    '<pre><code>this is
code
</code></pre>'

try 'mismatched backticks' '```tick``' '<p><code>`tick</code></p>'
try 'mismatched backticks(2)' '``tick```' '<p>``tick```</p>'
try 'unclosed single backtick' '`hi there' '<p>`hi there</p>'
try 'unclosed double backtick' '``hi there' '<p>``hi there</p>'
try 'triple backticks' '```hi there```' '<p><code>hi there</code></p>'
try 'quadruple backticks' '````hi there````' '<p><code>hi there</code></p>'
try 'remove space around code' '`` hi there ``' '<p><code>hi there</code></p>'
try 'code containing backticks' '`` a```b ``' '<p><code>a```b</code></p>'
try 'backslash before backtick' '`a\`' '<p><code>a\</code></p>'
try '`>`' '`>`' '<p><code>&gt;</code></p>'
try '`` ` ``' '`` ` ``' '<p><code>`</code></p>'
try '````` ``` `' '````` ``` `' '<p><code>``</code> `</p>'
try '````` ` ```' '````` ` ```' '<p><code>`` `</code></p>'
try 'backslashes in code(1)' '    printf "%s: \n", $1;' \
'<pre><code>printf "%s: \n", $1;
</code></pre>'
try 'backslashes in code(2)' '`printf "%s: \n", $1;`' \
'<p><code>printf "%s: \n", $1;</code></p>'

try 'fenced code disabled backtick' \
'```

unrecognized code!
```' \
'<p>```</p>

<p>unrecognized code!
```</p>'

# no <pre/> proves it’s the inline variant of `code` delimitation
try 'fenced code disabled backtick as inline code' \
'```
inline code?
```' \
'<p><code>
inline code?
</code></p>'

try 'fenced code disabled tilde' \
'~~~

unrecognized code!
~~~' \
'<p>~~~</p>

<p>unrecognized code!
~~~</p>'

try -ffencedcode 'fenced code block with blank lines' \
'~~~
code!

still code!
~~~' \
    '<pre><code>code!

still code!
</code></pre>'

try  -ffencedcode 'fenced code block' \
'~~~
code!
~~~' \
    '<pre><code>code!
</code></pre>'

try  -ffencedcode 'fenced code block in list' \
'1. ~~~
code block
~~~' \
'<ol>
<li><pre><code>code block
</code></pre></li>
</ol>'

try  -ffencedcode 'fenced code block in blockquote' \
'>~~~
code
~~~' \
'<blockquote><pre><code>code
</code></pre></blockquote>'

try  -ffencedcode 'unterminated fenced code block' \
'~~~
code' \
'<p>~~~
code</p>'

try  -ffencedcode 'fenced code block with tildes' \
'~~~~~
~~~
code with tildes
~~~
~~~~~' \
'<pre><code>~~~
code with tildes
~~~
</code></pre>'

try  -ffencedcode 'paragraph with trailing fenced block' \
'text text text
text text text
~~~
code code code?
~~~' \
'<p>text text text
text text text
~~~
code code code?
~~~</p>'

try  -ffencedcode 'fenced code blocks with backtick delimiters' \
'```
code
```' \
'<pre><code>code
</code></pre>'

try  -ffencedcode 'fenced code block with mismatched delimters' \
'```
code
~~~' \
'<p>```
code
~~~</p>'

try  -ffencedcode 'fenced code block with lang attribute' \
'```lang
code
```' \
'<pre><code class="lang">code
</code></pre>'

try  -ffencedcode 'fenced code block with lang-name attribute' \
'```lang-name
code
```' \
'<pre><code class="lang-name">code
</code></pre>'

try  -ffencedcode 'fenced code block with lang_name attribute' \
'```lang_name
code
```' \
'<pre><code class="lang_name">code
</code></pre>'

try  -ffencedcode 'fenced code block with lang attribute and space' \
'``` lang
code
```' \
'<pre><code class="lang">code
</code></pre>'

try  -ffencedcode 'fenced code block with lang attribute and multiple spaces' \
'```       lang
code
```' \
'<pre><code class="lang">code
</code></pre>'

try  -ffencedcode 'fenced code block with lang-name attribute and space' \
'``` lang-name
code
```' \
'<pre><code class="lang-name">code
</code></pre>'

try  -ffencedcode 'fenced code block with lang_name attribute and space' \
'``` lang_name
code
```' \
'<pre><code class="lang_name">code
</code></pre>'

summary $0
exit $rc
