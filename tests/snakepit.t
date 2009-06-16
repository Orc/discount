./echo "The snakepit of Markdown.pl compatability"

rc=0
MARKDOWN_FLAGS=

try() {
    
    S=`./echo -n "$1" '..................................' | ./cols 34`
    ./echo -n "  $S "

    Q=`./echo "$2" | ./markdown`
    count=`./echo "$Q" | grep "$3" | wc -l`

    if [ $count -eq 1 ]; then
	./echo "ok"
    else
	./echo "FAILED"
        ./echo "wanted $3"
        ./echo "got    $Q"
	rc=1
    fi
}

try '[](single quote) text (quote)' \
    "[foo](http://Poe's law)  will make this fail ('no, it won't!') here."\
    '<p><a href="http://Poe" title="s law)  will make this fail ('"'no, it won't!"'">foo</a> here'

try '[](unclosed <url)' '[foo](<http://no trailing gt)' \
			'<p><a href="http://no%20trailing%20gt">foo</a></p>'
exit $rc
