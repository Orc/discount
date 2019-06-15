/*
 * Pre-digested configuration header for MSVC on Windows.
 */

#ifndef __AC_MARKDOWN_D
#define __AC_MARKDOWN_D 1

#ifndef _MSC_VER
#error Use this header with MSVC only.
#endif

#define OS_WIN32 1

/*
 * `discount` feature macros - we want them all!
 */
#ifndef WITH_ID_ANCHOR
#define WITH_ID_ANCHOR 1
#endif
#ifndef WITH_FENCED_CODE
#define WITH_FENCED_CODE 1
#endif
#ifndef WITH_GITHUB_TAGS
#define WITH_GITHUB_TAGS 1
#endif
#ifndef USE_DISCOUNT_DL
#define USE_DISCOUNT_DL 1
#endif
#ifndef USE_EXTRA_DL
#define USE_EXTRA_DL 1
#endif

/*
 * The Visual C++ "C" compiler has a `__inline` keyword implemented
 * in Visual Studio 2008 and later, see
 * <http://msdn.microsoft.com/de-de/library/cx3b23a3%28v=vs.90%29.aspx>
 */
#if _MSC_VER >= 1500 /* VC 9.0, MSC_VER 15, Visual Studio 2008 */
#define inline __inline
#else
#define inline 
#endif

#ifdef _MSC_VER
#ifndef strncasecmp
#include <string.h>
#define bzero(p, n) memset(p, 0, n)
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp
#endif
#endif

/*
 * Beware of conflicts with <Windows.h>, which typedef's these names.
 */
#ifndef WINVER
#define DWORD unsigned long
#define WORD  unsigned short
#define BYTE  unsigned char
#endif

#define HAVE_PWD_H 0
#define HAVE_GETPWUID 0
#define HAVE_SRANDOM 0
#define INITRNG(x) srand((unsigned int)x)
#define HAVE_BZERO 0
#define HAVE_RANDOM 0
#define COINTOSS() (rand()&1)
#define HAVE_STRCASECMP  1
#define HAVE_STRNCASECMP 1
#define HAVE_FCHDIR 0
#define TABSTOP 8
#define HAVE_MALLOC_H    0

#define DESTRUCTOR

#endif /* __AC_MARKDOWN_D */
