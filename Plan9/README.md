# *Discount* Markdown compiler on Plan 9

## Build

### One line

    % mk install
    % markdown -V
    markdown: discount X.Y.Z GHC=INPUT

### Stepwise

    % CONFIG='--with-tabstops=7' mk config
    % mk
    % mk test
    % mk install
    % markdown -V
    markdown: discount X.Y.Z TAB=7 GHC=INPUT

See `../configure.sh` and `../pgm_options.c` for other config options.

### Other *mk*(1) targets

* `clean`: Delete built objects from source directory.
* `nuke`: Delete built objects and generated configuration.
* `install.libs`:  Discount includes a C library and header.
Installation is optional.  Plan 9 binaries are statically linked.
* `install.man`:  Add *markdown* in manual sections 1, 2, and 6.
* `install.progs`:  Extra programs.  *makepage* writes complete
XHTML documents, rather than fragments.  *mkd2html* is similar,
but produces HTML.
* `installall`: Do all `install*` targets above.
* `uninstall`: Remove anything added by `install*` targets above.

## Notes

This is not a port from POSIX to native Plan 9 APIs. The supplied
`mkfile` drives Discount's own `../configure.sh` and the `../Makefile`
it generates through [Plan 9's ANSI/POSIX Environment (APE)][ape-paper]
(in [*pcc*(1)][pcc-man]) to build the Discount source, then copies
the results to locations appropriate for system-wide use on Plan 9.

[ape-paper]: https://plan9.io/sys/doc/ape.html
[pcc-man]: https://plan9.io/magic/man2html/1/pcc
