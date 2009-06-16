./echo "Bugs & misfeatures reported by Mike Schiraldi"

rc=0
MARKDOWN_FLAGS=

try() {
    
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

try -fnohtml 'breaks with -fnohtml' 'foo  ' '<p>foo<br/>
</p>'

try 'links with trailing \)' \
    '[foo](http://en.wikipedia.org/wiki/Link_(film\))' \
    '<p><a href="http://en.wikipedia.org/wiki/Link_(film)">foo</a></p>'

try -fautolink '(url) with -fautolink' \
    '(http://tsfr.org)' \
    '<p>(<a href="http://tsfr.org">http://tsfr.org</a>)</p>'

try 'single #' \
    '#' \
    '<p>#</p>'

try '\( escapes in []()' \
    '[foo](http://a.com/\(foo\))' \
    '<p><a href="http://a.com/(foo)">foo</a></p>'

try -frelax '* processing with -frelax' \
    '2*4 = 8 * 1 = 2**3' \
    '<p>2*4 = 8 * 1 = 2**3</p>'

try -fnopants '[]() with a single quote mark' \
    '[Poe'"'"'s law](http://rationalwiki.com/wiki/Poe'"'"'s_Law)' \
    '<p><a href="http://rationalwiki.com/wiki/Poe'"'"'s_Law">Poe'"'"'s law</a></p>'

exit $rc
