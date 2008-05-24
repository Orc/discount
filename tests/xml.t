./echo "xml output with MKD_CDATA"

rc=0
MARKDOWN_FLAGS=

./echo -n '  xml output from markdown() ....... '

if ./echo '"hello,sailor"' | ./markdown -fcdata | grep '&amp;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi


./echo -n '  html output from markdown() ...... '

if ./echo '"hello,sailor"' | ./markdown -fnocdata | grep '&amp;' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi


./echo -n '  xml output from mkd_text() ....... '

if ./markdown -fcdata -t'"hello,sailor"' | grep '&amp;' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  html output from mkd_text() ...... '

if ./markdown -fnocdata -t'"hello,sailor"' | grep '&amp;' >/dev/null; then
    ./echo "FAILED"
    rc=1
else
    ./echo "ok"
fi

exit $rc
