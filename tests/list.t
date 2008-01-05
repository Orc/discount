echo "lists"

rc=0

echo -n '  two separated items .............. '

SEP=' * A

* B'

count=`echo "$SEP" | ./markdown | grep -i '<ul>' | wc -l`

if [ "$count" -eq 1 ]; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi

echo -n '  two adjacent items ............... '

SEP=' * A
 * B'

count=`echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq 0 ]; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi

echo -n '  two adjacent items, then space ... '

SEP=' * A
* B

space, the final frontier'

count=`echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq 1 ]; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi

echo -n '  nested lists (1) ................. '

SEP=' * A (list)
     1. Sub (list)
     2. Two (items)
     3. Here
 * B (list)'

count=`echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq 0 ] ; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi
echo -n '  nested lists (2) ................. '

SEP=' * A (list)
     1. Sub (list)
     2. Two (items)
     3. Here

     Here
 * B (list)'

count=`echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq 1 ] ; then
    echo "OK"
else
    echo "FAILED"
    rc=1
fi

exit $rc
