./echo "paranoia"

rc=0
MARKDOWN_FLAGS=

./echo -n '  bogus url (-fsafelink) ........... '

TRICK='[test](bad:protocol)'

count=`./echo "$TRICK" | ./markdown -fsafelink | grep '<a' | wc -l`

if [ $count -eq 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  bogus url (-fnosafelink) ......... '

TRICK='[test](bad:protocol)'

count=`./echo "$TRICK" | ./markdown -fnosafelink | grep '<a' | wc -l`

if [ $count -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
