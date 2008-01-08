echo "definition lists"

rc=0

SEP='
=this=
    is an ugly
=test=
    eh?
'

RES=`echo "$SEP" | ./markdown`

if ./markdown -V | grep DL_TAG >/dev/null; then

    echo -n '  =tag= generates definition lists . '

    count=`echo "$RES" | grep -i '<dl>' | wc -l`

    if [ $count -eq 1 ]; then
	echo "ok"
    else
	echo "FAILED"
	rc=1
    fi

    echo -n '  two item list has two labels ..... '

    count=`echo "$RES" | grep -i '<dt>' | wc -l`

    if [ $count -eq 2 ]; then
	echo "ok"
    else
	echo "FAILED"
	rc=1
    fi

    echo -n '  two item list has two entries..... '

    count=`echo "$RES" | grep -i '<dd>' | wc -l`

    if [ $count -eq 2 ]; then
	echo "ok"
    else
	echo "FAILED"
	rc=1
    fi

else
    echo -n '  =tag= does nothing ............... '

    count=`echo "$RES" | grep -i '<dl>' | wc -l`

    if [ $count -gt 0 ]; then
	echo "ok"
    else
	echo "FAILED"
	rc=1
    fi
fi

exit $rc
