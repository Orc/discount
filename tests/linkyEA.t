. tests/functions.sh

title "links with extended attributes"

rc=0

try -fextended_attr 'image with extended attribute' \
    '![picture](pic){rel=_nofollow}' \
    '<p><img src="pic" rel=_nofollow alt="picture" /></p>'

try -fextended_attr 'link with extended attribute' \
    '[link](link){rel=_nofollow}' \
    '<p><a href="link" rel=_nofollow>link</a></p>'

summary $0
exit $rc
