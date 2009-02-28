./echo "markdown 1.0 compatability"

rc=0
MARKDOWN_FLAGS=

LINKY='[this] is a test

[this]: /this'

./echo -n '  implicit reference links ......... '

RES=`./echo "$LINKY" | ./markdown`

count=`./echo "$RES" | grep -i '<a' | wc -l`

if [ "$count" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  implicit reference links (-f1.0) . '

RES=`./echo "$LINKY" | ./markdown -f1.0`

count=`./echo "$RES" | grep -i '<a' | wc -l`

if [ "$count" -eq 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

WSP=' '
WHITESPACE="
    white space$WSP
    and more"

./echo -n '  trailing whitespace .............. '

RES=`echo "$WHITESPACE" | ./markdown`

count=`echo "$RES" | grep -i ' $' | wc -l`

if [ $count -eq 1 ]; then
    ./echo "ok"
else
    ./echo "failed"
    rc=1
fi

./echo -n '  trailing whitespace (-f1.0) ...... '

RES=`echo "$WHITESPACE" | ./markdown -f1.0`

count=`echo "$RES" | grep -i ' $' | wc -l`

if [ $count -eq 0 ]; then
    ./echo "ok"
else
    ./echo "failed ($count)"
    rc=1
fi

exit $rc
