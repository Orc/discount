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
try 'mail address' '<orc@pell.portland.or.us>' '<a href='
try 'mail address with mailto:' '<mailto:orc@pell>' '<a href='
try 'invalid address (orc@)' '<orc@>' '<p>&lt;orc@></p>'
try 'invalid address (@pell)' '<@pell>' '<p>&lt;@pell></p>'
try 'invalid address (orc@pell)' '<orc@pell>' '<p>&lt;orc@pell></p>'
try 'invalid address (mailto:orc@)' '<mailto:orc@>' '<a href='
try 'invalid address (mailto:@pell)' '<mailto:@pell>' '<a href='

exit $rc
