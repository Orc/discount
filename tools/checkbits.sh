#! /bin/sh

trap "rm -f in.markdown.h in.mkdio.h" EXIT

grep 'MKD_' markdown.h | sed -ne '1,/}/p' > in.markdown.h
grep 'MKD_' mkdio.h | sed -ne '1,/}/p' > in.mkdio.h

diff -c -bw in.markdown.h in.mkdio.h
retcode=$?

[ $retcode -eq 0 ] || echo "markdown flags differ between markdown.h & mkdio.h!" 1>&2

exit $retcode
