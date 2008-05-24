./echo "definition lists"

rc=0
MARKDOWN_FLAGS=

SEP='
=this=
    is an ugly
=test=
    eh?
'

RES=`./echo "$SEP" | ./markdown`

if ./markdown -V | grep DL_TAG >/dev/null; then

    ./echo -n '  =tag= generates definition lists . '

    count=`./echo "$RES" | grep -i '<dl>' | wc -l`

    if [ $count -eq 1 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  two item list has two labels ..... '

    count=`./echo "$RES" | grep -i '<dt>' | wc -l`

    if [ $count -eq 2 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  two item list has two entries..... '

    count=`./echo "$RES" | grep -i '<dd>' | wc -l`

    if [ $count -eq 2 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  one item with two =tags= ......... '

    SRC="
=this=
=is=
    A test, eh?"

    RES=`./echo "$SRC" | ./markdown`

    count1=`./echo "$RES" | grep -i '<dt>' | wc -l`
    count2=`./echo "$RES" | grep -i '<dd>' | wc -l`

    if [ \( "${count1:-0}" -eq 2 \) -a \( "${count2:-0}" -eq 1 \) ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

else
    ./echo -n '  =tag= does nothing ............... '

    count=`./echo "$RES" | grep -i '<dl>' | wc -l`

    if [ $count -eq 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
fi

exit $rc
