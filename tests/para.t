./echo "paragraph blocking"

rc=0
MARKDOWN_FLAGS=

./echo -n '  paragraph followed by code ....... '

SEP="a
    b"

RES=`./echo "$SEP" | ./markdown`

pcount=`./echo "$RES" | grep -i '<p>' | wc -l`
ccount=`./echo "$RES" | grep -i '<code>' | wc -l`

if [ "$ccount" -eq 1 -a "$pcount" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  single-line paragraph ............ '

if ./echo "a" | ./markdown | grep '<p>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
