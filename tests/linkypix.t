./echo "embedded images"

rc=0
MARKDOWN_FLAGS=

./echo -n '  image with size extension ........ '

if ./echo '![picture](pic =200x200)' | ./markdown | grep -i 'width=' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
