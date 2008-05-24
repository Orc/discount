./echo "lists"

rc=0
MARKDOWN_FLAGS=

./echo -n '  two separated items (1) .......... '

SEP=' * A

* B'

RES=`./echo "$SEP" | ./markdown`

count=`./echo "$RES" | grep -i '<ul>' | wc -l`

if [ "$count" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  two separated items (2) .......... '

count=`./echo "$RES" | grep -i '<p>' | wc -l`

case `expr $count` in
2)  ./echo "ok" ;;
1)  ./echo "FAILED (known bug)" ;;
*)  ./echo "FAILED ($count)" 
    rc=1 ;;
esac


./echo -n '  two adjacent items ............... '

SEP=' * A
 * B'

count=`./echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  two adjacent items, then space ... '

SEP=' * A
* B

space, the final frontier'

count=`./echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  nested lists (1) ................. '

SUB='
 1. Sub (list)
 2. Two (items)
 3. Here'

SEP='
 *   1. Sub (list)
     2. Two (items)
     3. Here'

count1=`./echo "$SUB" | ./markdown | grep -i '<p>' | wc -l`
count=`./echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -eq "$count1" ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi
./echo -n '  nested lists (2) ................. '

SUB='
>A (list)
>
>1. Sub (list)
>2. Two (items)
>3. Here'

SEP='
 * A (list)

     1. Sub (list)
     2. Two (items)
     3. Here

     Here
 * B (list)'

count1=`./echo "$SUB" | ./markdown | grep -i '<p>' | wc -l`
count=`./echo "$SEP" | ./markdown | grep -i '<p>' | wc -l`

if [ "$count" -gt $count1 ] ; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  blockquote inside list ........... '

SEP='
 *  A (list)
   
    > quote
    > me

    dont quote me'

if ./echo "$SEP" | ./markdown | grep 'blockquote' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
