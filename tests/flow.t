./echo "paragraph flow"

rc=0
MARKDOWN_FLAGS=

./echo -n '  header followed by paragraph ..... '

SEP='###Hello, sailor###
And how are you today?'

RES=`./echo "$SEP" | ./markdown`

hcount=`./echo "$RES" | grep -i '<h3>' | wc -l`
pcount=`./echo "$RES" | grep -i '<p>' | wc -l`

if [ "$hcount" -eq 1 -a "$pcount" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  two lists punctuated with a HR ... '

SEP='* A
* * *
* B
* C'

RES=`./echo "$SEP" | ./markdown`

hrcount=`./echo "$RES" | grep -i '<hr' | wc -l`
ulcount=`./echo "$RES" | grep -i '<ul>' | wc -l`


if [ "$hrcount" -eq 1 -a "$ulcount" -eq 2 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
