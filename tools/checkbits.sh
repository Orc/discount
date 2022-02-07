#! /bin/sh

trap "rm -f in.markdown.h in.mkdio.h in.diff_markdown_mkdio.log" EXIT

grep 'MKD_' markdown.h | sed -ne '1,/}/p' > in.markdown.h
grep 'MKD_' mkdio.h | sed -ne '1,/}/p' > in.mkdio.h

diff -c -bw in.markdown.h in.mkdio.h 2> in.diff_markdown_mkdio.log
retcode=$?

# Retry without '-c' option for BusyBox at least v1.34
# that does not support it and returns '1' for the option error.
if [ $retcode -ne 0 ]; then
        if grep -iq "BusyBox" in.diff_markdown_mkdio.log; then
                echo "--- stderr log w/o -c option ---" >> in.diff_markdown_mkdio.log
                diff -bw in.markdown.h in.mkdio.h      2>> in.diff_markdown_mkdio.log; retcode=$?
                echo "--------------------------------" >> in.diff_markdown_mkdio.log
        fi
fi

# If still not '0'
[ $retcode -eq 0 ] || (cat in.diff_markdown_mkdio.log; echo "markdown flags differ between markdown.h & mkdio.h!" 1>&2)

exit $retcode
