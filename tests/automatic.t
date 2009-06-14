./echo "automatic links"

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

try 'http url' '<http://here>' '<a href="http://here">http://here</a>'
try 'ftp url' '<ftp://here>' '<a href="ftp://here">ftp://here</a>'
try '<orc@pell.portland.or.us>' '<orc@pell.portland.or.us>' '<a href='
try '<orc@pell.com.>' '<orc@pell.com.>' '<a href='
try 'invalid <orc@>' '<orc@>' '<p>&lt;orc@></p>'
try 'invalid <@pell>' '<@pell>' '<p>&lt;@pell></p>'
try 'invalid <orc@pell>' '<orc@pell>' '<p>&lt;orc@pell></p>'
try 'invalid <orc@.pell>' '<orc@.pell>' '<p>&lt;orc@.pell></p>'
try 'invalid <orc@pell.>' '<orc@pell.>' '<p>&lt;orc@pell.></p>'
try '<mailto:orc@pell>' '<mailto:orc@pell>' '<a href='
try '<mailto:orc@pell.com>' '<mailto:orc@pell.com>' '<a href='
try '<mailto:orc@>' '<mailto:orc@>' '<a href='
try '<mailto:@pell>' '<mailto:@pell>' '<a href='

exit $rc
