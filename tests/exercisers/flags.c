#include <stdio.h>
#include <mkdio.h>
#include <stdlib.h>

void
say(char *what)
{
    fputs(what,stdout);
    fflush(stdout);
}


int
main(void)
{
    mkd_flag_t *flags = mkd_flags();


    say("check flag functions: ");
    say("set ");
    mkd_set_flag_num(flags, MKD_DLDISCOUNT);

    say("set(0) ");
    mkd_set_flag_num(0, MKD_DLDISCOUNT);

    say("clear ");
    mkd_clr_flag_num(flags, MKD_DLDISCOUNT);

    say("clear(0) ");
    mkd_clr_flag_num(0, MKD_DLDISCOUNT);

    say("copy ");
    (void)mkd_copy_flags(flags);

    say("copy(0) ");
    (void)mkd_copy_flags(0);

    say("ok\n");
    exit(0);
}
