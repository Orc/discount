./echo "html blocks"

rc=0
MARKDOWN_FLAGS=

./echo -n '  self-closing block tags (hr) ..... '

SEP='
<hr>

text
'

count=`./echo "$SEP" | ./markdown | grep '<p>' | wc -l`

if [ $count -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  self-closing block tags (hr/) .... '

SEP='
<hr/>

text
'

count=`./echo "$SEP" | ./markdown | grep '<p>' | wc -l`

if [ $count -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  self-closing block tags (br) ..... '

SEP='
<br>

text
'

count=`./echo "$SEP" | ./markdown | grep '<p>' | wc -l`

if [ $count -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  html comments .................... '

SEP='
<!--
**hi**
-->
'

count=`./echo "$SEP" | ./markdown | grep 'strong' | wc -l`

if [ $count -eq 0 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  no smartypants inside tags (#1) .. '

count=`./echo '<img src="linky">' | ./markdown | tr -dc '"' | wc -c`

if [ $count -eq 2 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  no smartypants inside tags (#2) .. '

count=`./echo '<img src="linky" alt=":)" />' | ./markdown | tr -dc '"' | wc -c`

if [ $count -eq 4 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  block html with -fnohtml ......... '

RSLT=`./echo "<b>hi!</b>" | ./markdown -fnohtml`

if ./echo "$RSLT" | grep '<b>' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

./echo -n '  allow html with -fhtml ........... '

RSLT=`./echo "<b>hi!</b>" | ./markdown -fhtml`

if ./echo "$RSLT" | grep '<b>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
