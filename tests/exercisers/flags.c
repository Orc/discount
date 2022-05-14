#include <stdio.h>
#include <mkdio.h>
#include <stdlib.h>

int
main()
{
    mkd_flag_t *flags = mkd_flags();


    fprintf(stderr, "check flag functions: ");
    fprintf(stderr, "set ");
    mkd_set_flag_num(flags, MKD_DLDISCOUNT);

    fprintf(stderr, "set(0) ");
    mkd_set_flag_num(0, MKD_DLDISCOUNT);

    fprintf(stderr, "clear ");
    mkd_clr_flag_num(flags, MKD_DLDISCOUNT);

    fprintf(stderr, "clear(0) ");
    mkd_clr_flag_num(0, MKD_DLDISCOUNT);

    fprintf(stderr, "copy ");

    (void)mkd_copy_flags(flags);

    fprintf(stderr, "copy(0) ");

    (void)mkd_copy_flags(0);

    fprintf(stderr, "ok\n");
    exit(0);
}
