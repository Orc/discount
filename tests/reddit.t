./echo "bugs & misfeatures found during the Reddit rollout"

rc=0
MARKDOWN_FLAGS=

try() {
    unset FLAGS
    case "$1" in
    -*) FLAGS=$1
	shift ;;
    esac
    
    S=`./echo -n "$1" '..................................' | ./cols 34`
    ./echo -n "  $S "

    Q=`./echo "$2" | ./markdown $FLAGS`


    if [ "$3" = "$Q" ]; then
	./echo "ok"
    else
	./echo "FAILED"
	./echo "wanted: $3"
	./echo "got   : $Q"
	rc=1
    fi
}

try 'smiley faces?' '[8-9] <]:-( x ---> [4]' \
		    '<p>[8-9] &lt;]:&ndash;( x &mdash;&ndash;> [4]</p>'

try 'really long ETX headers' \
    '#####################################################hi' \
    '<h6>###############################################hi</h6>'

try 'unescaping "  " inside `code`' \
'`foo  
bar`' \
'<p><code>foo  
bar</code></p>'

try 'unescaping "  " inside []()' \
'[foo](bar  
bar)' \
'<p><a href="bar  %0Abar">foo</a></p>'

exit $rc
