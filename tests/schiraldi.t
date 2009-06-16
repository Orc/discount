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

try -fautolink 'autolink url with escaped spaces' \
    'http://\(here\ I\ am\)' \
    '<p><a href="http://(here%20I%20am)">http://(here I am)</a></p>'

try -fautolink 'autolink café_racer' \
    'http://en.wikipedia.org/wiki/café_racer' \
    '<p><a href="http://en.wikipedia.org/wiki/caf%C3%A9_racer">http://en.wikipedia.org/wiki/caf%C3%A9_racer</a></p>'

try -fautolink 'autolink url with arguments' \
    'http://foo.bar?a&b=c' \
    '<p><a href="http://foo.bar?a&amp;b=c">http://foo.bar?a&amp;b=c</a></p>'

try -fautolink 'autolink url with escaped ()' \
    'http://a.com/\(foo\)' \
    '<p><a href="http://a.com/(foo)">http://a.com/(foo)</a></p>'

try -fautolink 'autolink url with trailing \' \
    'http://a.com/\\\)' \
    '<p><a href="http://a.com/\)">http://a.com/\)</a></p>'


exit $rc
