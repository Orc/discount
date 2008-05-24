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

exit $rc
