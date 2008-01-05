echo "misc"

rc=0

echo -n '  single paragraph ................. '

RES=`echo AAA | ./markdown`

count=`echo "$RES" | grep -i '<p>' | wc -l`

if [ "$count" -eq 1 ]; then
    echo "OK"
else
    echo "Failed"
    rc=1
fi

exit $rc
