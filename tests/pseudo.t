./echo "pseudo-protocols"

rc=0
MARKDOWN_FLAGS=

./echo -n '  [](id:) links .................... '

ASK=`./echo '[foo](id:bar)' | ./markdown`
if ./echo "$ASK" | grep '<a id="bar">foo</a>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  [](id:) links with MKD_NO_EXT .... '

ASK=`./echo '[foo](id:bar)' | MARKDOWN_FLAGS=0x0040 ./markdown`
if ./echo "$ASK" | grep '<a id="bar">foo</a>' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

./echo -n '  [](class:) links ................. '

ASK=`./echo '[foo](class:bar)' | ./markdown`

if ./echo "$ASK" | grep '<span class="bar">foo</span>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  [](class:) links with MKD_NO_EXT . '

ASK=`./echo '[foo](class:bar)' | MARKDOWN_FLAGS=0x0040 ./markdown`

if ./echo "$ASK" | grep '<span class="bar">foo</span>' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

./echo -n '  [](raw:) links ................... '

ASK=`./echo '[foo](raw:bar)' | ./markdown`

count1=`./echo "$ASK" | grep foo | wc -l`
count2=`./echo "$ASK" | grep bar | wc -l`

if [ "$count1" -eq 0 -a "$count2" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  [](raw:) links with MKD_NO_EXT ... '

ASK=`./echo '[foo](raw:bar)' | MARKDOWN_FLAGS=0x0040 ./markdown`

count1=`./echo "$ASK" | grep 'raw:bar' | wc -l`
count2=`./echo "$ASK" | grep 'a href' | wc -l`

if [ \( "$count1" -gt 0 \) -a \( "$count2" -gt 0 \) ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
