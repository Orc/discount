# NMAKE Makefile to build Discount with Visual C++
CFLAGS	=	/nologo /MP /MDd /EHa /Zi \
			/D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /D_SCL_SECURE_NO_WARNINGS \
			/I.
LIBOBJ	=	mkdio.obj markdown.obj dumptree.obj generate.obj \
			resource.obj docheader.obj version.obj toc.obj css.obj \
			xml.obj Csio.obj xmlpage.obj basename.obj emmatch.obj \
			github_flavoured.obj setup.obj tags.obj html5.obj flags.obj
MKDLIB	= libmarkdown.lib
PGMS=markdown
SAMPLE_PGMS=mkd2html makepage
# modules that markdown, makepage, mkd2html, &tc use
COMMON=pgm_options.obj gethopt.obj

# Assumes VERSION is a single-line file
VERSION = \
	!INCLUDE <VERSION>

default:	all

all:	$(MKDLIB) $(PGMS) $(SAMPLE_PGMS) 

$(MKDLIB):	$(LIBOBJ)
	if exist $(MKDLIB) del $(MKDLIB)
	lib /out:$(MKDLIB) $(LIBOBJ)

config.h: msvc/config.h.vc
	copy /Y msvc\config.h.vc config.h

version.c: version.c.in
	powershell.exe -Command "(gc version.c.in) -replace '@TABSTOP@', 'TABSTOP' | Out-File version.c"

version.obj: version.c VERSION config.h
	$(CC) $(CFLAGS) -DBRANCH="" -DVERSION=\"$(VERSION)\" /c version.c

mkdio.h: mkdio.h.in
	powershell.exe -Command "(gc mkdio.h.in) -replace '@DWORD@', 'unsigned long' -replace '@SCALAR_HEADER_INCLUDE@', '' | Out-File mkdio.h"

mkdio.obj: mkdio.h

tags.obj: tags.c cstring.h tags.h blocktags

pgm_options.obj: pgm_options.c mkdio.h config.h
	$(CC) $(CFLAGS) /c pgm_options.c

mktags: mktags.obj
	$(CC) $(CFLAGS) mktags.obj

blocktags: mktags
	.\mktags.exe > blocktags

mkd2html:  mkd2html.obj $(MKDLIB) mkdio.h gethopt.h gethopt.obj notspecial.obj
	$(CC) $(CFLAGS) $(LFLAGS) mkd2html.obj gethopt.obj notspecial.obj $(MKDLIB)

markdown: main.obj $(COMMON) $(MKDLIB)
	$(CC) $(CFLAGS) $(LFLAGS) /Femarkdown main.obj $(COMMON) $(MKDLIB)

makepage:  makepage.c $(COMMON) $(MKDLIB) mkdio.h
	$(CC) $(CFLAGS) $(LFLAGS) makepage.c $(COMMON) $(MKDLIB)

clean:
	-del config.h blocktags mkdio.h version.c
	-del *.obj *.lib
	-del *.pdb *.exp
	-del *.ilk *.exe
