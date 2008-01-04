echo "crashes"

rc=0

echo -n '  hanging quote in list ............ '

./markdown >/dev/null 2>/dev/null << EOF
 * > this should not die

no.
EOF

if [ "$?" -eq 0 ]; then
    echo "OK"
else
    echo "Failed"
    rc=1
fi

echo -n '  dangling list item ............... '

if echo ' - ' | ./markdown >/dev/null 2>/dev/null; then
    echo "OK"
else
    echo "Failed"
    rc=1
fi
