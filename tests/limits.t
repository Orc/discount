. tests/functions.sh

title 'recursion limits'
rc=0

limits() {
    try_header "$1"

    res=`./300 400 $3 $4 $5 | ./markdown | tr -dc "$2" | wc -c`

    if (( $res > 0 )); then
	# it worked
	test $VERBOSE && ./echo " ok"
    else
	test "$VERBOSE" && ./echo "failed"
	rc=1
    fi
    
}


limits "superscripts" "()" "2^(" "A" ")"

limits "list items" "1" "1. " "item"

limits "blockquotes" ">" "> " "item"

summary $0
exit $rc
