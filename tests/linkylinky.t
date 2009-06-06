./echo "embedded links"

rc=0
MARKDOWN_FLAGS=

./echo -n '  url contains & ................... '

if ./echo '[hehehe](u&rl)' | ./markdown | grep -i '&amp;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  url contains + ................... '

if ./echo '[hehehe](u+rl)' | ./markdown | grep -i '+' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  url contains " ................... '

if ./echo '[hehehe](u"rl)' | ./markdown | grep -i '%22' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

./echo -n '  url contains < ................... '

if ./echo '[hehehe](u<rl)' | ./markdown | grep -i '&lt;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  url contains whitespace .......... '

if ./echo '[hehehe](r u)' | ./markdown | grep -i '"r%20u"' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  url contains whitespace & title .. '

if ./echo '[hehehe](r u "there")' | ./markdown | grep -i '"r%20u"' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  url contains escaped ) ........... '

if ./echo '[hehehe](u\))' | ./markdown | grep -i '"u)"' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  image label contains < ........... '

if ./echo '![he<he<he](url)' | ./markdown | grep -i '&lt;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  image label contains > ........... '

if ./echo '![he>he>he](url)' | ./markdown | grep -i '&gt;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  sloppy context link .............. '

if ./echo '[heh]( url "how about it?" )' | ./markdown | grep -i '</a>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  footnote urls formed properly .... '

TEST='[hehehe]: hohoho "ha ha"

[hehehe][]
'

if ./echo "$TEST" | ./markdown | grep -i '&#00;' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

./echo -n '  linky-like []s work .............. '

if ./echo '[foo]' | ./markdown | fgrep '[foo]' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  pseudo-protocol "id:" ............ '

if ./echo '[foo](id:bar)' | ./markdown | fgrep 'a id' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  pseudo-protocol "class:" ......... '

if ./echo '[foo](class:bar)' | ./markdown | fgrep 'span class' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  pseudo-protocol "abbr:" .......... '

if ./echo '[foo](abbr:bar)' | ./markdown | fgrep '<abbr title="bar">' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  nested [][]s ..................... '

count=`./echo '[[z](y)](x)' | ./markdown | tr '>' '\n' | grep -i '<a href' | wc -l`

if [ "$count" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  empty [][] tags .................. '

V="
[![][1]][2]

[1]: image1
[2]: image2"

count=`echo "$V" | ./markdown | fgrep '[]' | wc -l`

if [ "$count" -lt 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
