./echo "backslash escapes"

rc=0
MARKDOWN_FLAGS=

try() {
    
    S=`./echo -n "$1" '..................................' | cut -c 1-34`
    ./echo -n "  $S "

    Q=`./echo "$2" | ./markdown`
    count=`./echo "$Q" | grep -F "$3" | wc -l`

    if [ $count -eq 1 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
}

try 'backslashes in []()' '[foo](http://\this\is\.a\test\(here\))' \
'<p><a href="http://\this\is.a\test(here)">foo</a></p>'

exit $rc
