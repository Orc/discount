. tests/functions.sh

title "locale fuckery"

rc=0
MARKDOWN_FLAGS=

run() {
    echo "$1" | ./markdown $2
}

batch() {
    run "[x](/a—)"
    run "# a ⚠" -ftoc
    run "# a ⚠" -ftoc,urlencodedanchor
}


try_header 'output for $LANG=C vs $LANG=en_US.UTF8'

compare() {
    try_header "$1"

    LANG="C" batch > $$C
    LANG="en_US.UTF-8" batch > $$UTF

    if cmp $$C $$UTF; then
	__passed=`expr $__passed + 1`
	test $VERBOSE && ./echo " ok"
    else
	__failed=`expr $__failed + 1`
	if [ -z "$VERBOSE" ]; then
	    ./echo
	    ./echo "$1"
	fi
	./echo "diff:"
	diff $$C $$UTF | sed - 's/^/	/'
	rc=1
    fi
    rm -f $$C $$UTF
}


compare 'output with LANG=C vs LANG=en_US.UTF8'

summary $0
exit $rc
