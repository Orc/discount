echo "lists"

rc=0

echo -n '  two separated items .............. '

SEP=' * A

* B'

size=`echo "$SEP" | ./markdown | grep -i '<ul>' | wc -l`

if [ "$size" -eq 1 ]; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi

echo -n '  two adjacent items ............... '

SEP=' * A
 * B'

size=`echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$size" -eq 0 ]; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi

exit $rc
