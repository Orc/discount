echo "emphasis"

rc=0
MARKDOWN_FLAGS=

echo -n '  *hi* -> <em>hi</em> .............. '

if echo '*hi*' | ./markdown | grep -i '<em>hi</em>' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  * -> * ........................... '

if echo 'A * A' | ./markdown | grep -i 'A \* A' > /dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

exit $rc
