./echo "footnotes inside reparse sections"

rc=0

RES="[![foo][]](bar)

[foo]: bar2"

./echo -n '  footnote inside [] section ....... '

if ./echo "$RES" | ./markdown | fgrep bar2 >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

exit $rc
