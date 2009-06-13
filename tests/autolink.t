./echo 'Reddit-style embedded links'
rc=0

./echo -n '  single link ...................... '

V="http://www.pell.portland.or.us/~orc/Code/discount"
Q=`echo "$V" | ./markdown -fautolink | grep -i '<a href=' | wc -l`

if [ ${Q:-0} -eq 1 ]; then
    ./echo "ok"
else
    ./echo "FAILED"
    rc=1
fi

./echo -n '  link surrounded by text .......... '

V="here http://it is?"
Q=`echo "$V" | ./markdown -fautolink`

case "$Q" in
'<p>here <a href="http://it">http://it</a> is?</p>') ./echo "ok" ;;
*)	./echo "FAILED"
	rc=1;;
esac

./echo -n '  naked @ .......................... '

V="@"
Q=`echo "$V" | ./markdown -fautolink`

case "$Q" in
'<p>@</p>') ./echo "ok" ;;
*)	./echo "FAILED"
	rc=1;;
esac

./echo -n '  (url) ............................ '

Q=`echo "(http://here)" | ./markdown -fautolink`

case "$Q" in
'<p>(<a href="http://here">http://here</a>)</p>') ./echo "ok" ;;
*)	./echo "FAILED"
	rc=1;;
esac

./echo -n '  token with trailing @ ............ '

Q=`echo "orc@" | ./markdown -fautolink`

case "$Q" in
'<p>orc@</p>') ./echo "ok" ;;
*)             ./echo "FAILED";
	       rc=1;;
esac

exit $rc
