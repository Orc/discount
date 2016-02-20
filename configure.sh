#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

# load in the configuration file
#
ac_help='--enable-amalloc	Enable memory allocation debugging
--with-tabstops=N	Set tabstops to N characters (default is 4)
--with-latex		Enable latex passthrough
--enable-all-features	Turn on all stable optional features
--shared		Build shared libraries (default is static)'

LOCAL_AC_OPTIONS='
set=`locals $*`;
if [ "$set" ]; then
    eval $set
    shift 1
else
    ac_error=T;
fi'

locals() {
    K=`echo $1 | $AC_UPPERCASE`
    case "$K" in
    --SHARED)
                echo TRY_SHARED=T
                ;;
    --ENABLE-ALL|--ENABLE-ALL-FEATURES)
		echo WITH_AMALLOC=T
		;;
    --ENABLE-*)	enable=`echo $K | sed -e 's/--ENABLE-//' | tr '-' '_'`
		echo WITH_${enable}=T ;;
    --DEBIAN-GLITCH)
		echo DEBIAN_GLITCH=T
		;;
    esac
}

TARGET=markdown
. ./configure.inc

AC_INIT $TARGET

for banned_with in dl fenced-code id-anchor github-tags urlencoded-anchor; do
    banned_with_variable_ref=\$WITH_`echo "$banned_with" | $AC_UPPERCASE | tr - _`
    if [ "`eval echo "$banned_with_variable_ref"`" ]; then
	LOG "Setting theme default --with-$banned_with."
    fi
done

# theme wants the old behavior of --with-(foo)
#
case "`echo "$WITH_DL" | $AC_UPPERCASE`" in
    EXTRA)         THEME_CF="MKD_DLEXTRA|MKD_NODLDISCOUNT";;
    BOTH)          THEME_CF="MKD_DLEXTRA";;
esac
test "$WITH_FENCED_CODE" && THEME_CF="${THEME_CF:+$THEME_CF|}MKD_FENCEDCODE"

AC_DEFINE THEME_CF "$THEME_CF"


test "$DEBIAN_GLITCH" && AC_DEFINE 'DEBIAN_GLITCH' 1

AC_PROG_CC

test "$TRY_SHARED" && AC_COMPILER_PIC && AC_CC_SHLIBS

if [ "IS_BROKEN_CC" ]; then
    case "$AC_CC $AC_CFLAGS" in
    *-pedantic*) ;;
    *)  # hack around deficiencies in gcc and clang
	#
	AC_DEFINE 'while(x)' 'while( (x) != 0 )'
	AC_DEFINE 'if(x)' 'if( (x) != 0 )'

	if [ "$IS_CLANG" ]; then
	    AC_CC="$AC_CC -Wno-implicit-int"
	elif [ "$IS_GCC" ]; then
	    AC_CC="$AC_CC -Wno-return-type -Wno-implicit-int"
	fi ;;
    esac
fi

AC_PROG ar || AC_FAIL "$TARGET requires ar"
AC_PROG ranlib

AC_C_VOLATILE
AC_C_CONST
AC_C_INLINE
AC_SCALAR_TYPES sub hdr
AC_CHECK_BASENAME
AC_CHECK_ALLOCA

AC_CHECK_HEADERS sys/types.h pwd.h && AC_CHECK_FUNCS getpwuid

if AC_CHECK_FUNCS srandom; then
    AC_DEFINE 'INITRNG(x)' 'srandom((unsigned int)x)'
elif AC_CHECK_FUNCS srand; then
    AC_DEFINE 'INITRNG(x)' 'srand((unsigned int)x)'
else
    AC_DEFINE 'INITRNG(x)' '(void)1'
fi

if AC_CHECK_FUNCS 'bzero((char*)0,0)'; then
    : # Yay
elif AC_CHECK_FUNCS 'memset((char*)0,0,0)'; then
    AC_DEFINE 'bzero(p,s)' 'memset(p,s,0)'
else
    AC_FAIL "$TARGET requires bzero or memset"
fi

if AC_CHECK_FUNCS random; then
    AC_DEFINE 'COINTOSS()' '(random()&1)'
elif AC_CHECK_FUNCS rand; then
    AC_DEFINE 'COINTOSS()' '(rand()&1)'
else
    AC_DEFINE 'COINTOSS()' '1'
fi

if AC_CHECK_FUNCS strcasecmp; then
    :
elif AC_CHECK_FUNCS stricmp; then
    AC_DEFINE strcasecmp stricmp
else
    AC_FAIL "$TARGET requires either strcasecmp() or stricmp()"
fi

if AC_CHECK_FUNCS strncasecmp; then
    :
elif AC_CHECK_FUNCS strnicmp; then
    AC_DEFINE strncasecmp strnicmp
else
    AC_FAIL "$TARGET requires either strncasecmp() or strnicmp()"
fi

if AC_CHECK_FUNCS fchdir || AC_CHECK_FUNCS getcwd ; then
    AC_SUB 'THEME' ''
else
    AC_SUB 'THEME' '#'
fi

if [ -z "$WITH_TABSTOPS" ]; then
    TABSTOP=4
elif [ "$WITH_TABSTOPS" -eq 1 ]; then
    TABSTOP=8
else
    TABSTOP=$WITH_TABSTOPS
fi
AC_DEFINE 'TABSTOP' $TABSTOP
AC_SUB    'TABSTOP' $TABSTOP


if [ "$WITH_AMALLOC" ]; then
    AC_DEFINE	'USE_AMALLOC'	1
    AC_SUB	'AMALLOC'	'amalloc.o'
else
    AC_SUB	'AMALLOC'	''
fi

[ "$OS_FREEBSD" -o "$OS_DRAGONFLY" ] || AC_CHECK_HEADERS malloc.h

[ "$WITH_PANDOC_HEADER" ] && AC_DEFINE 'PANDOC_HEADER' '1'
[ "$WITH_LATEX" ] && AC_DEFINE 'WITH_LATEX' '1'

AC_OUTPUT Makefile version.c mkdio.h
