./echo "footnotes"

rc=0
MARKDOWN_FLAGS=

./echo -n '  a line with multiple []s ......... '

SZ=`./echo '[a][] [b][]:' | ./markdown | wc -l`

if [ "$SZ" -gt 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  a valid footnote ................. '

FMT='
[alink][]

[alink]: link me'

SZ=`./echo "$FMT" | ./markdown | grep '<a href' | wc -l`

if [ "$SZ" -gt 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
