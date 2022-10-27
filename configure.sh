#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

# load in the configuration file
#
ac_help='--enable-amalloc	Enable memory allocation debugging
--with-tabstops=N	Set tabstops to N characters (default is 4)
--shared		Build shared libraries (default is static)
--pkg-config		Install pkg-config(1) glue files
--cxx-binding		Install header files with c++ wrappers
--github-checkbox[=input] Enable github-style checkboxes in lists
			(if =input, use <input checkbox>, otherwise
			use html ballot entities)'

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
    --ENABLE-*)	enable=`echo $K | sed -e 's/--ENABLE-//' | tr '-' '_'`
		echo WITH_${enable}=T ;;
    --DEBIAN-GLITCH)
		echo DEBIAN_GLITCH=T
		;;
    --H1-TITLE)
		echo H1TITLE=T
		;;
    --PKG-CONFIG)
		echo PKGCONFIG=T
		;;
    --CXX-BINDING)
		echo CXX_BINDING=T
		;;
    --GITHUB-CHECKBOX=ENTITY)
		echo GITHUB_CHECKBOX_STYLE=entity
		;;
    --GITHUB-CHECKBOX=INPUT)
		echo GITHUB_CHECKBOX_STYLE=input
		;;
    esac
}

VERSION=`cat VERSION`
TARGET=markdown
. ./configure.inc

# if there's a makefile here, it's likely that it's a discount
# makefile and there's bits of an old configuration here.  So
# blow everything away before we start the configuration.

test -f Makefile && make spotless 2>/dev/null >/dev/null

AC_INIT $TARGET
AC_SUB 'PACKAGE_NAME' lib$TARGET
AC_SUB 'PACKAGE_VERSION' $VERSION

# define definition list type defaults (for theme)
#
case "`echo "$WITH_DL" | $AC_UPPERCASE`" in
    DISCOUNT)      AC_DEFINE THEME_DL_MODE 1 ;;
    EXTRA)         AC_DEFINE THEME_DL_MODE 2 ;;
    BOTH)          AC_DEFINE THEME_DL_MODE 3 ;;
esac
test "$WITH_FENCED_CODE" && AC_DEFINE THEME_FENCED_CODE 1

AC_DEFINE THEME_CF "$THEME_CF"


test "$DEBIAN_GLITCH" && AC_DEFINE 'DEBIAN_GLITCH' 1

AC_PROG_CC
AC_QUIET AC_PROG git && AC_DEFINE 'HAS_GIT' '1'
AC_CHECK_ATTRIBUTE destructor

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

# should we create a .pc for pkg-config & GNU automake
#
if [ "$PKGCONFIG" ]; then
    AC_SUB MK_PKGCONFIG ''
elif AC_PROG pkg-config || AC_PROG automake ; then
    PKGCONFIG=true
    AC_SUB MK_PKGCONFIG ''
else
    AC_SUB MK_PKGCONFIG '#'
fi

AC_C_VOLATILE
AC_C_CONST
AC_C_INLINE
AC_SCALAR_TYPES sub hdr
AC_CHECK_BASENAME
AC_CHECK_ALLOCA

AC_CHECK_HEADERS sys/types.h pwd.h && AC_CHECK_FUNCS getpwuid
if AC_CHECK_HEADERS sys/stat.h && AC_CHECK_FUNCS stat; then

# need to check some of the S_ISxxx stat macros, because they may not
# exist (for notspecial.c)

cat > ngc$$.c << EOF
#include <sys/stat.h>

int main(int argc, char **argv)
{
   struct stat info;

    if ( stat(argv[0], &info) != 0 )
	return 1;

    return MACRO(info.st_mode);
}
EOF
    LOGN "special file macros in sys/stat.h:"
    _none="none"
    for x in ISSOCK ISCHR ISFIFO; do
	if $AC_CC -DMACRO=S_$x -o ngc$$.o  ngc$$.c; then
	    LOGN " S_${x}"
	    AC_DEFINE "HAS_${x}" '1'
	    unset _none
	fi
    done
    LOG "${_none}."
    __remove ngc$$.o ngc$$.c
fi

if AC_CHECK_FUNCS srandom; then
    AC_DEFINE 'INITRNG(x)' 'srandom((unsigned int)x)'
elif AC_CHECK_FUNCS srand; then
    AC_DEFINE 'INITRNG(x)' 'srand((unsigned int)x)'
else
    AC_DEFINE 'INITRNG(x)' '(void)1'
fi

AC_CHECK_FUNCS 'memset((char*)0,0,0)' 'string.h' || \
	    AC_CHECK_FUNCS 'memset((char*)0,0,0)' || \
		      AC_FAIL "$TARGET requires memset"

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

if [ "$H1TITLE" ]; then
    AC_SUB 'H1TITLE' h1title.o
    AC_DEFINE USE_H1TITLE 1
else
    AC_SUB 'H1TITLE' ''
fi

if [ "$GITHUB_CHECKBOX_STYLE" = "entity" ]; then
    AC_DEFINE 'CHECKBOX_AS_INPUT' '0'
else
    AC_DEFINE 'CHECKBOX_AS_INPUT' '1'
fi


[ "$OS_FREEBSD" -o "$OS_DRAGONFLY" ] || AC_CHECK_HEADERS malloc.h

[ "$WITH_PANDOC_HEADER" ] && AC_DEFINE 'PANDOC_HEADER' '1'

GENERATE="Makefile version.c mkdio.h"

if [ "$PKGCONFIG" ]; then
    GENERATE="$GENERATE libmarkdown.pc"
fi

AC_OUTPUT $GENERATE

if [ "$CXX_BINDING" ]; then
    LOG "applying c++ glue to mkdio.h"
    mv mkdio.h mkdio.h$$
    (   echo '#ifdef __cplusplus'
	echo 'extern "C" {'
	echo '#endif'
	cat mkdio.h$$
	echo '#ifdef __cplusplus'
	echo '}'
	echo '#endif' ) > mkdio.h
    rm mkdio.h$$
fi
