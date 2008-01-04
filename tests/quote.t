echo -n "quote................................"

exec 2> /dev/null

if ./markdown tests/quote.text >/dev/null; then
    echo "OK"
    exit 0
fi
echo "FAILED"
exit 1
