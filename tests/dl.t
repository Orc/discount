. tests/functions.sh

title "definition lists"

rc=0
MARKDOWN_FLAGS=

SRC='
=this=
    is an ugly
=test=
    eh?'

RSLT='<dl>
<dt>this</dt>
<dd>is an ugly</dd>
<dt>test</dt>
<dd>eh?</dd>
</dl>'

# discount style
try -fdefinitionlist '=tag= generates definition lists' "$SRC" "$RSLT"

try 'one item with two =tags=' \
    '=this=
=is=
    A test, eh?' \
	    '<dl>
<dt>this</dt>
<dt>is</dt>
<dd>A test, eh?</dd>
</dl>'

# extra style
try -fnodefinitionlist,dlextra '=tag= does nothing' "$SRC" \
    '<p>=this=
    is an ugly
=test=
    eh?</p>'

try -fdlextra 'markdown extra-style definition lists' \
'foo
: bar' \
'<dl>
<dt>foo</dt>
<dd>bar</dd>
</dl>'

try -fdlextra '... with two <dt>s in a row' \
'foo
bar
: baz' \
'<dl>
<dt>foo</dt>
<dt>bar</dt>
<dd>baz</dd>
</dl>'

try -fdlextra '... with two <dd>s in a row' \
'foo
: bar
: baz' \
'<dl>
<dt>foo</dt>
<dd>bar</dd>
<dd>baz</dd>
</dl>'

try -fdlextra '... with blanks between list items' \
'foo
: bar

zip
: zap' \
'<dl>
<dt>foo</dt>
<dd>bar</dd>
<dt>zip</dt>
<dd>zap</dd>
</dl>'

# Hmm, redundancy...
SRC='foo
: bar

=this=
    is ugly'
RSLT='<p>foo
: bar</p>

<p>=this=
    is ugly</p>'
try -fnodldiscount '... with definitionlists enabled but all styles disabled' \
"$SRC" \
"$RSLT"
try -fnodefinitionlist,dldiscount,dlextra '... with definitionlists disabled but all styles enabled' \
    "$SRC" \
    "$RSLT"

summary $0
exit $rc
