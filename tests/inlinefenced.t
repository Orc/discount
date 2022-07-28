. tests/functions.sh

title "inlined fenced code blocks"

try -ffencedinline 'inlined fenced code disabled backtick' \
'```

unrecognized code!
```' \
'<p>```</p>

<p>unrecognized code!
```</p>'

try -ffencedinline 'inlined fenced code disabled backtick as inline code' \
'```
inline code?
```' \
'<p><code>
inline code?
</code></p>'

try -ffencedinline 'inlined fenced code disabled tilde' \
'~~~

unrecognized code!
~~~' \
'<p>~~~</p>

<p>unrecognized code!
~~~</p>'

try -ffencedcode,fencedinline 'inlined fenced code block with blank lines' \
'~~~
code!

still code!
~~~' \
    '<p><pre><code>code!

still code!
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block' \
'~~~
code!
~~~' \
    '<p><pre><code>code!
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block in list' \
'1. ~~~
code block
~~~' \
'<ol>
<li><pre><code>code block
</code></pre>
</li>
</ol>'

try  -ffencedcode,fencedinline 'inlined fenced code block in blockquote' \
'>~~~
code
~~~' \
'<blockquote><p><pre><code>code
</code></pre>
</p></blockquote>'

try  -ffencedcode,fencedinline 'unterminated inlined fenced code block' \
'~~~
code' \
'<p>~~~
code</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with tildes' \
'~~~~~
~~~
code with tildes
~~~
~~~~~' \
'<p><pre><code>~~~
code with tildes
~~~
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'paragraph with trailing inlined fenced block' \
'text text text
text text text
~~~
code code code?
~~~' \
'<p>text text text
text text text
<pre><code>code code code?
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code blocks with backtick delimiters' \
'```
code
```' \
'<p><pre><code>code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with mismatched delimters' \
'```
code
~~~' \
'<p>```
code
~~~</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang attribute' \
'```lang
code
```' \
'<p><pre><code class="lang">code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang-name attribute' \
'```lang-name
code
```' \
'<p><pre><code class="lang-name">code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang_name attribute' \
'```lang_name
code
```' \
'<p><pre><code class="lang_name">code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang attribute and space' \
'``` lang
code
```' \
'<p><pre><code class="lang">code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang attribute and multiple spaces' \
'```       lang
code
```' \
'<p><pre><code class="lang">code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang-name attribute and space' \
'``` lang-name
code
```' \
'<p><pre><code class="lang-name">code
</code></pre>
</p>'

try  -ffencedcode,fencedinline 'inlined fenced code block with lang_name attribute and space' \
'``` lang_name
code
```' \
'<p><pre><code class="lang_name">code
</code></pre>
</p>'

try -ffencedcode,fencedinline 'inlined fenced code block with blank line in the middle' \
'```
hello

sailor
```' \
'<p><pre><code>hello

sailor
</code></pre>
</p>'


try -ffencedcode,fencedinline 'inlined fenced code block with html in the middle' \
'~~~~
<h1>hello, sailor</h1>
~~~~' \
'<p><pre><code>&lt;h1&gt;hello, sailor&lt;/h1&gt;
</code></pre>
</p>'

try -ffencedcode,fencedinline 'inlined fenced code block with trailing spaces in list item' \
'1.  ~~~~    
    test me
    ~~~~' \
'<ol>
<li><pre><code>test me
</code></pre>
</li>
</ol>'

try -ffencedcode,fencedinline 'inlined unterminated fenced code block' \
'~~~~
foo' \
'<p>~~~~
foo</p>'

try -ffencedcode,fencedinline 'paragraph, then inlined code block' \
'foo

~~~~
bar
~~~~' \
'<p>foo</p>

<p><pre><code>bar
</code></pre>
</p>'


try -ffencedcode,fencedinline 'checkline misparse as inlined fenced code' \
'[`label`](#code)
```class
content
```
' \
'<p><a href="#code"><code>label</code></a>
<pre><code class="class">content
</code></pre>
</p>'


try -ffencedcode,fencedinline 'subitem inlined fenced code' \
'+ item
  ```
  code
  ```' \
'<ul>
<li>item
<pre><code>code
</code></pre>
</li>
</ul>'


try -ffencedcode,fencedinline 'subitem inlined fenced code with class' \
'+ item
  ```class
  code
  ```' \
'<ul>
<li>item
<pre><code class="class">code
</code></pre>
</li>
</ul>'


summary $0
exit $rc
