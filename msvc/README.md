# *Discount* Markdown compiler on Windows with Visual C++

Makefile for NMAKE utility to build Discount using Visual C++
and pre-digested `config.h` template.

## Build

Build generates default optimised binaries.
Edit `CFLAGS` in `Makefile` to customize generated binaries (eg. switch to debug build).

* Generate static library `libmarkdown.lib` and utility programs:

    nmake /f msvc/Makefile

* Clean source tree

    nmake /f msvc/Makefile clean
