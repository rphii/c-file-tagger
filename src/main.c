//usr/bin/env tcc -DPROC_COUNT=4 $(ls *.c | grep -v main.c) -run "$0" "$@" ; exit $?

#include <stdio.h>

#include "file.h"
#include "lookup.h"
#include "platform.h"
#include "err.h"
#include "arg.h"
#include "str.h"
#include "info.h"
#include "cft.h"


#include <linux/limits.h>

//#include <ctype.h>

int main(int argc, const char **argv)
{
    int err = 0;
    info_disable_all(INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_file, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_skip_incorrect_extension, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_skip_too_large, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_directory, INFO_LEVEL_ALL);
    //info_enable_all(INFO_LEVEL_ID | INFO_LEVEL_TEXT);
    
    TRYC(platform_colorprint_init());

    Arg arg = {0};
    Cft cft = {0};
    Str ostream = {0};

    TRY(arg_parse(&arg, argc > 0 ? (size_t)argc : 0, argv), ERR_ARG_PARSE);
    if(arg.exit_early) goto clean;
    //screen_enter();

    /* program start */
    TRYC(cft_arg(&cft, &arg));
    TRYC(cft_init(&cft));

    /* read specified file */
    //TRYC(file_str_read(&arg.parsed.file, &cft.parse.content));
    if(file_size(&arg.parsed.file) != SIZE_MAX) {
        TRYC(file_exec(&arg.parsed.file, &cft.parse.dirfiles, cft.options.recursive, cft_parse_file, &cft));
    }

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
            TRYC(file_exec(input, &cft.parse.dirfiles, cft.options.recursive, cft_parse_file, &cft));
            while(vstr_length(&cft.parse.dirfiles)) {
                vstr_pop_back(&cft.parse.dirfiles, input);
                memset(cft.parse.dirfiles.items[vstr_length(&cft.parse.dirfiles)], 0, sizeof(Str)); // TODO: this should probably happen in my vector!
                TRYC(file_exec(input, &cft.parse.dirfiles, cft.options.recursive, cft_parse_file, &cft));
                str_free(input);
            }
        }
    }

    /* reformat */
    if(cft.options.modify || cft.options.merge) {
        if(!str_length(&arg.parsed.file)) {
            THROW("no %s provieded", arg_str(ARG_OUTPUT));
        }
        TRYC(cft_tags_add(&cft, &arg.parsed.remains, &arg.parsed.tags_add));
        //printff("RE [%.*s]", STR_F(&arg.parsed.tags_re));
        TRYC(cft_tags_re(&cft, &arg.parsed.remains, &arg.parsed.tags_re));
        str_clear(&cft.parse.content);
        TRYC(cft_fmt(&cft, &cft.parse.content));
        TRYC(file_str_write(&arg.parsed.file, &cft.parse.content));
        goto clean;
    }

    /* query files */
    if(cft.options.query) {
        //printff("any [%.*s]", STR_F(&arg.parsed.find_any));
        //printff("and [%.*s]", STR_F(&arg.parsed.find_and));
        //printff("not [%.*s]", STR_F(&arg.parsed.find_not));
        TRYC(cft_find_fmt(&cft, &ostream, &arg.parsed.find_any, &arg.parsed.find_and, &arg.parsed.find_not));
        printf("%.*s", STR_F(&ostream));
        goto clean;
    }

    if(str_length(&arg.parsed.substring_tags)) {
        TRYC(cft_fmt_substring_tags(&cft, &ostream, &arg.parsed.substring_tags));
        printf("%.*s", STR_F(&ostream));
        goto clean;
    }

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
    cft_free(&cft);
    arg_free(&arg);
    info_handle_abort();
    //screen_leave();
    return err;
error:
    ERR_CLEAN;
        }

