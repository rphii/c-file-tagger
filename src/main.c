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

    TRY(arg_parse(&arg, argc > 0 ? (size_t)argc : 0, argv), ERR_ARG_PARSE);
    if(arg.exit_early) goto clean;
    //screen_enter();

    /* program start */
    TRYC(cft_init(&cft));
    TRYC(cft_arg(&cft, &arg));

    /* read specified file */
    TRYC(file_str_read(&arg.parsed.file, &content));
    TRYC(cft_parse(&cft, &content));

    /* read all other specified files */
    if(!cft.options.modify || cft.options.merge) {
        for(size_t i = 0; i < vrstr_length(&arg.parsed.inputs); ++i) {
            Str *input = vrstr_get_at(&arg.parsed.inputs, i);
            if(!str_cmp(input, &arg.parsed.file)) {
                /* TODO add some info here that skips two exact file paths... doesn't
                 * break anything if we DO, but helps to inform the user about what
                 * he's doing (absolutely not happened to me and I did NOT get confused
                 * by it) besides, expanding paths (.. / ~ / . etc) exists, sooo... */
                continue;
            }
            str_clear(&content);
            TRYC(file_str_read(input, &content));
            TRYC(cft_parse(&cft, &content));
        }
    }

    /* reformat */
    if(cft.options.modify || cft.options.merge) {
        TRYC(cft_tags_add(&cft, &arg.parsed.remains, &arg.parsed.tags_add));
        str_clear(&content);
        TRYC(cft_fmt(&cft, &content));
        TRYC(file_str_write(&arg.parsed.file, &content));
        goto clean;
    }

    /* query files */
    if(cft.options.query) {
        TRYC(cft_find_fmt(&cft, &ostream, &arg.parsed.find_any, &arg.parsed.find_and, &arg.parsed.find_not));
        printf("%.*s", STR_F(&ostream));
        goto clean;
    }

    //printff("list-tags %i", cft.options.list_tags);
    //printff("list-files %i", cft.options.list_files);
    //return 0;

    /* print files */
    if(cft.options.list_files && cft.options.list_files > cft.options.list_tags) {
        TRYC(cft_files_fmt(&cft, &ostream, &arg.parsed.remains));
        printf("%.*s", STR_F(&ostream));
        goto clean;
    }

    /* print tags */
    if(cft.options.list_tags && cft.options.list_tags > cft.options.list_files) {
        TRYC(cft_tags_fmt(&cft, &ostream, &arg.parsed.remains));
        printf("%.*s", STR_F(&ostream));
        goto clean;
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

