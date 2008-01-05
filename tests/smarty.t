echo "smarty pants"

rc=0

echo -n '  (c) -> &copy; .................... '

if  echo '(c)' | ./markdown | grep '&copy;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  (r) -> &reg; ..................... '

if  echo '(r)' | ./markdown | grep '&reg;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  (tm) -> &trade; .................. '

if  echo '(tm)' | ./markdown | grep '&trade;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  ... -> &hellip; .................. '

if  echo '...' | ./markdown | grep '&hellip;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  -- -> &mdash; .................... '

if  echo '--' | ./markdown | grep '&mdash;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  - -> &ndash; ..................... '

if  echo 'regular - ' | ./markdown | grep '&ndash;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  A-B -> A-B ....................... '

if  echo 'A-B' | ./markdown | grep '&ndash;' >/dev/null; then
    echo "FAILED"
    rc=1
else
    echo "ok"
fi

echo -n '  "fancy" -> &ldquo;fancy&rdquo; ... '

if  echo '"fancy"' | ./markdown | grep '&ldquo;fancy&rdquo;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  '"'fancy'"' -> &lsquo;fancy&rsquo; ... '

if  echo "'fancy'" | ./markdown | grep '&lsquo;fancy&rsquo;' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n "  don't -> don't ................... "

if  echo "don't" | ./markdown | grep "don't" >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n '  `` ` `` -> <code> ` </code> ...... '

if  echo '`` ` ``' | ./markdown | grep '<code> ` </code>' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi

echo -n "  it's -> it&rsquo;s ............... "

if  echo "it's" | ./markdown | grep 'it&rsquo;s' >/dev/null; then
    echo "ok"
else
    echo "FAILED"
    rc=1
fi


exit $rc
