# *Discount* Markdown compiler on Plan 9

## Build
    % CONFIG='--with-tabstops=7' mk config
    % mk test
    % mk install
    % markdown -V
    markdown: discount X.Y.Z TAB=7

### Configuration
To select features and extensions, `--with-tabstops=7` may be replaced by zero or more of:

* `--enable-pandoc-header`:  Use pandoc-style header blocks
* `--enable-superscript`:  `A^B` becomes A<sup>B</sup>
* `--enable-amalloc`:  Enable memory allocation debugging
* `--with-tabstops=`*N*:  Set tabstops to *N* characters (default 4)
* `--enable-alpha-list`:  Enable `(a)/(b)/(c)` list markers
* `--enable-all-features`:  Turn on all stable optional features

## Notes
1. This is not a port from POSIX to native Plan 9 APIs. The supplied
`mkfile` merely drives Discount's own `configure.sh` through Plan 9's
*APE* environment (in *pcc*(1)) to build the Discount source, then
copies the result to locations appropriate for system-wide use on
Plan 9.

2. There are a few other *mk*(1) targets:
    * `install.libs`:  Discount includes a C library and header.
Installation is optional.  Plan 9 binaries are statically linked.
    * `install.man`:  Add manual pages for *markdown* in sections 1, 2, and 6.
    * `install.progs`:  Extra programs.  *makepage* writes complete XHTML
documents, rather than fragments.  *mkd2html* is similar, but produces
HTML.
