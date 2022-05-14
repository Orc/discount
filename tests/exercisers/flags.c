#include <stdio.h>
#include <mkdio.h>
#include <stdlib.h>

int
main()
{
    mkd_flag_t *flags = mkd_flags();
    mkd_flag_t *copy;


    fprintf(stderr, "exercise flags: ");
    fprintf(stderr, "set ");
    mkd_set_flag_num(flags, MKD_DLDISCOUNT);

    fprintf(stderr, "clear ");
    mkd_clr_flag_num(flags, MKD_DLDISCOUNT);

    fprintf(stderr, "copy ");

    copy = mkd_copy_flags(flags);

    fprintf(stderr, "ok\n");
    exit(0);
}
