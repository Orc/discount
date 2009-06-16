./echo "backslash escapes"

rc=0
MARKDOWN_FLAGS=

try() {
    
    S=`./echo -n "$1" '..................................' | ./cols 34`
    ./echo -n "  $S "

    Q=`./echo "$2" | ./markdown`

    if [ "$3" = "$Q" ]; then
	./echo "ok"
    else
	./echo "FAILED"
	./echo "wanted: $3"
	./echo "got:    $Q"
	rc=1
    fi
}

try 'backslashes in []()' '[foo](http://\this\is\.a\test\(here\))' \
'<p><a href="http://\this\is.a\test(here)">foo</a></p>'

exit $rc
