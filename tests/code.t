./echo "code blocks"

rc=0
MARKDOWN_FLAGS=

./echo -n '  format for code block html ....... '

SEP='
    this is
    code
'

count=`echo "$SEP" | ./markdown | wc -l`

if [ $count -eq 3 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  unclosed single backtick ......... '

if ./echo '`hi there' | ./markdown | grep '`' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  unclosed double backtick ......... '

if ./echo '``hi there' | ./markdown | grep '``' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  remove space around code ......... '

if ./echo '`` hi there ``' | ./markdown | grep '<code>hi there<\/code>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
