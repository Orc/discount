#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

# load in the configuration file
#
ac_help='--enable-dl-tag	Use the DL tag extension
--enable-pandoc-header	Use pandoc-style header blocks'

LOCAL_AC_OPTIONS='
set=`locals $*`;
if [ "$set" ]; then
    eval $set
    shift 1
else
    ac_error=T;
fi'

locals() {
    K=`echo $1 | tr '[a-z]' '[A-Z]'`
    case "$K" in
    --ENABLE-*)	enable=`echo $K | sed -e 's/--ENABLE-/WITH-/' | tr '-' '_'`
		echo ${enable}=T
		;;
    esac
}

TARGET=markdown
. ./configure.inc

set >log

AC_INIT $TARGET

AC_PROG_CC

case "$AC_CC $AC_CFLAGS" in
*-Wall*)    AC_DEFINE 'while(x)' 'while( (x) != 0 )'
	    AC_DEFINE 'if(x)' 'if( (x) != 0 )' ;;
esac

AC_PROG ar || AC_FAIL "$TARGET requires ar"
AC_PROG ranlib

AC_C_VOLATILE
AC_C_CONST
AC_SCALAR_TYPES
AC_CHECK_ALLOCA || AC_FAIL "$TARGET requires alloca()"

AC_CHECK_FUNCS basename
AC_CHECK_HEADERS libgen.h

[ "$OS_FREEBSD" -o "$OS_DRAGONFLY" ] || AC_CHECK_HEADERS malloc.h

[ "$WITH_DL_TAG" ] && AC_DEFINE 'DL_TAG_EXTENSION' '1'
[ "$WITH_PANDOC_HEADER" ] && AC_DEFINE 'PANDOC_HEADER' '1'

AC_OUTPUT Makefile
