./echo "markup peculiarities"

rc=0
MARKDOWN_FLAGS=

./echo -n '  list followed by header .......... '

SRC="
- AAA
- BBB
-"

count=`./echo "$SRC" | ./markdown | sed -e '1,/<\/ul>/d' | wc -l`

if [ "$count" -gt 1 ]; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

exit $rc
