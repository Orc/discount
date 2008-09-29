./echo "headers"

rc=0
MARKDOWN_FLAGS=

try() {
    
    S=`./echo -n "$1" '..................................' | cut -c 1-34`
    ./echo -n "  $S "

    count=`./echo "$2" | ./markdown | grep "$3" | wc -l`

    if [ $count -eq 1 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
}

try 'empty ETX' '##' '<h1>#</h1>'
try 'single-char ETX (##W)' '##W' '<h2>W</h2>'
try 'single-char ETX (##W )' '##W  ' '<h2>W</h2>'
try 'single-char ETX (## W)' '## W' '<h2>W</h2>'
try 'single-char ETX (## W )' '## W ' '<h2>W</h2>'
try 'single-char ETX (##W##)' '##W##' '<h2>W</h2>'
try 'single-char ETX (##W ##)' '##W ##' '<h2>W</h2>'
try 'single-char ETX (## W##)' '## W##' '<h2>W</h2>'
try 'single-char ETX (## W ##)' '## W ##' '<h2>W</h2>'

try 'multiple-char ETX (##Hello##)' '##Hello##' '<h2>Hello</h2>'

exit $rc
