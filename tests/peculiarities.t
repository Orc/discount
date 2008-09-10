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

./echo -n '  ul with mixed item prefixes ...... '

SRC="
-  A
1. B
"

RES=`echo "$SRC" | ./markdown`

ulc=`./echo "$RES" | grep '<ul>' | wc -l`
olc=`./echo "$RES" | grep '<ol>' | wc -l`

if [ $ulc -eq 1 -a $olc -eq 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  ol with mixed item prefixes ...... '

SRC="
1. A
-  B
"

RES=`echo "$SRC" | ./markdown`

ulc=`./echo "$RES" | grep '<ul>' | wc -l`
olc=`./echo "$RES" | grep '<ol>' | wc -l`

if [ $olc -eq 1 -a $ulc -eq 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi


exit $rc
