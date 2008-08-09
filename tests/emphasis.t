./echo "emphasis"

rc=0
MARKDOWN_FLAGS=
MARKDOWN_VERSION=`./markdown -V`

./echo -n '  *hi* -> <em>hi</em> .............. '

if ./echo '*hi*' | ./markdown | grep -i '<em>hi</em>' >/dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  * -> * ........................... '

if ./echo 'A * A' | ./markdown | grep -i 'A \* A' > /dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  ***A**B* ......................... '

if ./echo '***A**B*' | ./markdown | grep -i '<em><strong>A</strong>B</em>' > /dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  ***A*B** ......................... '

if ./echo '***A*B**' | ./markdown | grep -i '<strong><em>A</em>B</strong>' > /dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  **A*B*** ......................... '

if ./echo '**A*B***' | ./markdown | grep -i '<strong>A<em>B</em></strong>' > /dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  *A**B*** ......................... '

if ./echo '*A**B***' | ./markdown | grep -i '<em>A<strong>B</strong></em>' > /dev/null; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  A_B .............................. '
case "$MARKDOWN_VERSION" in
*RELAXED*)
    if ./echo '_A_B' | ./markdown | grep -i 'A_B' > /dev/null; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
    ;;
*)  if ./echo '_A_B' | ./markdown | grep -i 'A</em>B' > /dev/null; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
    ;;
esac

exit $rc
