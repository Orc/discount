rc=0
unset MARKDOWN_FLAGS
unset MKD_TABSTOP


TS=`grep TABSTOP config.h | tr -dc '0-9'`

if [ "$TS" -eq 8 ]; then
    echo "dealing with tabstop derangement"

    LIST='
 *  A
     *  B
	 *  C'

    count1=`echo "$LIST" | ./markdown | grep -i '<ul>' | wc -l`
    count2=`echo "$LIST" | MKD_TABSTOP=Yuk ./markdown | grep -i '<ul>' | wc -l`

    echo -n '  using MKD_TABSTOP to break tabs .. '

    if [ "$count1" -ne "$count2" ]; then
	echo "ok"
    else
	echo "FAILED"
	rc=1
    fi
fi

exit $rc
