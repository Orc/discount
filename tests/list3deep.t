./echo "deeply nested lists"

rc=0
MARKDOWN_FLAGS=

./echo -n '  thrice-nested lists .............. '

LIST='
 *  top-level list ( list 1)
     +  second-level list (list 2)
        * first item third-level list (list 3)
     +  * second item, third-level list, first item. (list 4)
        * second item, third-level list, second item.
 *  top-level list again.'
     
count=`./echo "$LIST" | ./markdown | grep -i '<ul>' | wc -l`

if [ "$count" -eq 4 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
