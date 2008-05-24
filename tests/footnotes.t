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

exit $rc
