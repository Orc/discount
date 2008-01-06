echo "embedded links"

rc=0

echo -n '  url contains & ................... '

if echo '[hehehe](u&rl)' | ./markdown | grep -i '&amp;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  url contains " ................... '

if echo '[hehehe](u"rl)' | ./markdown | grep -i '%22' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  url contains < ................... '

if echo '[hehehe](u<rl)' | ./markdown | grep -i '&lt;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  label contains < ................. '

if echo '![he<he<he](url)' | ./markdown | grep -i '&lt;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  label contains " ................. '

if echo '![he"hehe](url)' | ./markdown | grep -i '&#34;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

exit $rc
