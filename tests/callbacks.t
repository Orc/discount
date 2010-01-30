./echo "callbacks"

rc=0
MARKDOWN_FLAGS=

try() {
    unset FLAGS
    case "$1" in
    -*) FLAGS=$1
	shift ;;
    esac
    
    ./echo -n "  $1" '..................................' | ./cols 36

    Q=`./echo "$2" | ./markdown $FLAGS`


    if [ "$3" = "$Q" ]; then
	./echo " ok"
    else
	./echo " FAILED"
	./echo "wanted: $3"
	./echo "got   : $Q"
	rc=1
    fi
}

try -bZZZ 'url modification' \
'[a](/b)' \
'<p><a href="ZZZ/b">a</a></p>'

try -EZZZ 'additional flags' \
'[a](/b)' \
'<p><a href="/b" ZZZ>a</a></p>'

exit $rc
