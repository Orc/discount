echo "html blocks"

rc=0
MARKDOWN_FLAGS=

echo -n '  self-closing block tags (hr) ..... '

SEP='
<hr>

text
'

count=`echo "$SEP" | ./markdown | grep '<p>' | wc -l`

if [ $count -eq 1 ]; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  self-closing block tags (hr/) .... '

SEP='
<hr/>

text
'

count=`echo "$SEP" | ./markdown | grep '<p>' | wc -l`

if [ $count -eq 1 ]; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  self-closing block tags (br) ..... '

SEP='
<br>

text
'

count=`echo "$SEP" | ./markdown | grep '<p>' | wc -l`

if [ $count -eq 1 ]; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  html comments .................... '

SEP='
<!--
**hi**
-->
'

count=`echo "$SEP" | ./markdown | grep 'strong' | wc -l`

if [ $count -eq 0 ]; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  no smartypants inside tags ....... '

count=`echo '<img src="linky">' | ./markdown | tr -dc '"' | wc -c`

if [ $count -eq 2 ]; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

exit $rc
