./echo "pandoc headers"

rc=0
MARKDOWN_FLAGS=

if ./markdown -V | grep HEADER > /dev/null; then

    ./echo -n '  valid header ..................... '
    TEXT='% title
% author(s)
% date
'
    count=`./echo "$TEXT" | ./markdown | grep title | wc -l`

    if [ "$count" -eq 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  valid header with -F0x0100........ '
    TEXT='% title
% author(s)
% date
'
    count=`./echo "$TEXT" | ./markdown -F0x0100 | grep title | wc -l`

    if [ "$count" -gt 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  invalid header ................... '
    TEXT='% title
% author(s)
a pony!'

    count=`./echo "$TEXT" | ./markdown | grep title | wc -l`

    if [ "$count" -gt 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  offset header .................... '
    TEXT='
% title
% author(s)
% date
content'

    count=`./echo "$TEXT" | ./markdown | grep title | wc -l`

    if [ "$count" -gt 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi

    ./echo -n '  indented header .................. '
    TEXT='    % title
    % author(s)
    % date
content'

    count=`./echo "$TEXT" | ./markdown | grep title | wc -l`

    if [ "$count" -gt 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
else
    ./echo -n '  ignore headers ................... '
    TEXT='% title
% author(s)
% date
'
    count=`./echo "$TEXT" | ./markdown | grep title | wc -l`

    if [ "$count" -gt 0 ]; then
	./echo "ok"
    else
	./echo "FAILED"
	rc=1
    fi
fi

exit $rc
