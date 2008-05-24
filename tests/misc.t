./echo "misc"

rc=0
MARKDOWN_FLAGS=

./echo -n '  single paragraph ................. '

RES=`./echo AAA | ./markdown`

count=`./echo "$RES" | grep -i '<p>' | wc -l`

if [ "$count" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  < -> &lt; ........................ '

if ./echo '<' | ./markdown | grep -i '&lt;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  `>` -> <code>&gt;</code> ......... '

if ./echo '`>`' | ./markdown | grep -i '<code>&gt;</code>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  `` ` `` -> <code>`</code> ........ '

if  ./echo '`` ` ``' | ./markdown | grep '<code>`</code>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
