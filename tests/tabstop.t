rc=0
unset MARKDOWN_FLAGS
unset MKD_TABSTOP


eval `./markdown -V | tr ' ' '\n' | grep TAB`

if [ "${TAB:-4}" -eq 8 ]; then
    ./echo "dealing with tabstop derangement"

    LIST='
 *  A
     *  B
	 *  C'

    count1=`./echo "$LIST" | ./markdown | grep -i '<ul>' | wc -l`
    count2=`./echo "$LIST" | MARKDOWN_FLAGS=0x0200 ./markdown | grep -i '<ul>' | wc -l`

    ./echo -n '  MARKDOWN_FLAGS breaks tabstops ... '

    if [ "$count1" -ne "$count2" ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
fi

exit $rc
