./echo "headers"

rc=0
MARKDOWN_FLAGS=

./echo -n '  single-character ETX headers ..... '

count=`./echo "## W" | ./markdown | grep '<h2>' | wc -l`

if [ $count -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
