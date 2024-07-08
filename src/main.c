//usr/bin/env tcc -DPROC_COUNT=4 $(ls *.c | grep -v main.c) -run "$0" "$@" ; exit $?

#include <stdio.h>

#include "file.h"
#include "lookup.h"
#include "platform.h"
#include "err.h"
#include "arg.h"
#include "screen.h"
//#include "colorprint.h"
#include "str.h"
#include "info.h"
#include "cft.h"


#include <linux/limits.h>

//#include <ctype.h>

int main(int argc, const char **argv)
{
    int err = 0;
    info_disable_all(INFO_LEVEL_ALL);
    //info_enable_all(INFO_LEVEL_ID | INFO_LEVEL_TEXT);
    TRY(platform_colorprint_init(), ERR_PLATFORM_COLORPRINT_INIT);

    Arg arg = {0};
    Cft cft = {0};
    Str content = {0};
    Str ostream = {0};

    TRY(arg_parse(&arg, argc, argv), ERR_ARG_PARSE);
    if(arg.exit_early) goto clean;
    //screen_enter();

    /* program start */

    /* read specified file */
    TRYC(cft_init(&cft));
    TRYC(cft_arg(&cft, &arg));
    TRYC(file_str_read(&arg.parsed.file, &content));
    TRYC(cft_parse(&cft, &content));

    /* reformat */
    if(0) {
        str_clear(&content);
        TRYC(cft_fmt(&cft, &content));
        Str output = STR("../data/tags-output.txt");
        TRYC(file_str_write(&output, &content));
    }

    /* print all tags */
    TRYC(cft_find_fmt(&cft, &ostream, &arg.parsed.find_any, &arg.parsed.find_and, &arg.parsed.find_not, arg.parsed.list_tags));
    printf("%.*s", STR_F(&ostream));

    if(!str_length(&ostream) && arg.parsed.list_tags) {
        TRYC(cft_tags_fmt(&cft, &ostream, &arg.parsed.remains));
        printf("%.*s", STR_F(&ostream));
    }

clean:
    str_free(&ostream);
    str_free(&content);
    cft_free(&cft);
    arg_free(&arg);
    info_handle_abort();
    //screen_leave();
    return err;
error:
    ERR_CLEAN;
}

