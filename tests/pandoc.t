. tests/functions.sh

title "pandoc headers"

rc=0
MARKDOWN_FLAGS=

HEADER='% title
% author(s)
% date'


try 'valid header' "$HEADER" ''
try -F0x00010000 'valid header with -F0x00010000' "$HEADER" '<p>% title
% author(s)
% date</p>'

try 'invalid header' \
	'% title
% author(s)
a pony!' \
	'<p>% title
% author(s)
a pony!</p>'

try 'offset header' \
	'
% title
% author(s)
% date' \
	'<p>% title
% author(s)
% date</p>'

try 'indented header' \
	'  % title
% author(s)
% date' \
	'<p>  % title
% author(s)
% date</p>'


# verify that empty pandoc header elements are handled properly

check_null() {
    try_header "$3"
    res=`echo "$2" | ./pandoc_headers $1`
    if [ "$res" ]; then
	./echo
	test $VERBOSE || ./echo "$3"
	echo "$res"
    else
	test $VERBOSE && ./echo " ok"
    fi
}

# pandoc headers left empty
EMPTY='%
%
%
stuff'

# pandoc headers with whitespace, but no content
EMPTY1='% 
% 
% 
stuff'

AUTHOR='%
% bofh
%
stuff'
TITLE='%bofh
%
%
stuff'
DATE='%
%
%now
stuff'

check_null -atd "$EMPTY" "empty"
check_null -atd "$EMPTY1" "whitespace"

check_null -td "$AUTHOR" "author field"
check_null -ad "$TITLE" "title field"
check_null -at "$DATE" "date field"

summary $0
exit $rc
