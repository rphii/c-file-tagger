//usr/bin/env tcc -DPROC_COUNT=4 $(ls *.c | grep -v main.c) -run "$0" "$@" ; exit $?

#include <stdio.h>

#include <rphii/file.h>
#include <rphii/str.h>
#include <rphii/arg.h>
#include "lookup.h"
#include "platform.h"
#include "info.h"
#include "cft.h"


#include <linux/limits.h>

//#include <ctype.h>

int *increment_list_tags(Cft *x) {
    ASSERT_ARG(x);
    if(x->options.list_files) ++x->options.list_files;
    ++x->options.list_tags;
    return 0;
}

int *increment_list_files(Cft *x) {
    ASSERT_ARG(x);
    if(x->options.list_tags) ++x->options.list_tags;
    ++x->options.list_files;
    return 0;
}

int *set_do_query(Cft *x) {
    ASSERT_ARG(x);
    x->options.query = true;
    return 0;
}

int main(int argc, const char **argv)
{
    int err = 0;

    //setvbuf(stdout, 0, _IOFBF, 0x10000);

    info_disable_all(INFO_LEVEL_ALL);
    //info_enable(INFO_tag_search, INFO_LEVEL_ALL);
    //info_enable(INFO_tag_create, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_file, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_skip_incorrect_extension, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_skip_too_large, INFO_LEVEL_ALL);
    //info_enable(INFO_parsing_directory, INFO_LEVEL_ALL);
    //info_enable(INFO_formatting, INFO_LEVEL_ALL);
    //info_enable_all(INFO_LEVEL_ID | INFO_LEVEL_TEXT);
    
    TRYC(platform_colorprint_init());

    Cft cft = {0};
    Str ostream = {0};

    bool exit_early = false;
    struct ArgX *x = 0;
    struct Arg *arg = arg_new();
    arg_init(arg, RSTR_L(argv[0]), RSTR("tag managing application"), RSTR(""));
    arg_init_rest(arg, RSTR("files|tags"), &cft.options.rest);
    x=argx_init(arg_opt(arg), 'h', RSTR("help"), RSTR("print this help"));
      argx_help(x, arg);
    x=argx_init(arg_opt(arg), 0 , RSTR("version"), RSTR("display the version"));
    x=argx_init(arg_opt(arg), 't', RSTR("tag"), RSTR("tag files"));
      argx_str(x, &cft.options.tags_add, 0);
    x=argx_init(arg_opt(arg), 0 , RSTR("retag"), RSTR("TBD rename files"));
      argx_str(x, &cft.options.tags_re, 0);
    x=argx_init(arg_opt(arg), 'u', RSTR("untag"), RSTR("TBD untag files"));
      argx_str(x, &cft.options.tags_del, 0);
    x=argx_init(arg_opt(arg), 'r', RSTR("recursive"), RSTR("recursively search subdirectories"));
      argx_bool(x, &cft.options.recursive, 0);
    x=argx_init(arg_opt(arg), 'O', RSTR("any"), RSTR("list files with any tags"));
      argx_str(x, &cft.options.find_any, 0);
      argx_func(x, 0, set_do_query, &cft, false);
    x=argx_init(arg_opt(arg), 'A', RSTR("and"), RSTR("list files having multiple tags"));
      argx_str(x, &cft.options.find_and, 0);
      argx_func(x, 0, set_do_query, &cft, false);
    x=argx_init(arg_opt(arg), 'N', RSTR("not"), RSTR("list files not having tags"));
      argx_str(x, &cft.options.find_not, 0);
      argx_func(x, 0, set_do_query, &cft, false);
    x=argx_init(arg_opt(arg), 'l', RSTR("list-tags"), RSTR("list all tags"));
      argx_func(x, 0, increment_list_tags, &cft, false);
    x=argx_init(arg_opt(arg), 'L', RSTR("list-files"), RSTR("list all files"));
      argx_func(x, 0, increment_list_files, &cft, false);
    x=argx_init(arg_opt(arg), 'T', RSTR("title"), RSTR("show title in output"));
      argx_bool(x, &cft.options.title, 0);
    x=argx_init(arg_opt(arg), 'd', RSTR("decorate"), RSTR("specify decoration"));
      argx_bool(x, &cft.options.decorate, 0);
    x=argx_init(arg_opt(arg), 'i', RSTR("input"), RSTR("specify additional input files"));
      argx_vstr(x, &cft.options.inputs, 0);
    x=argx_init(arg_opt(arg), 'o', RSTR("output"), RSTR("specify output file"));
      argx_str(x, &cft.options.output, 0);
      cft.options.argx.output = x;
    x=argx_init(arg_opt(arg), 'e', RSTR("expand-paths"), RSTR("expand paths"));
      argx_bool(x, &cft.options.expand_paths, 0);
    x=argx_init(arg_opt(arg), 'x', RSTR("extensions"), RSTR("specify extensions, comma seperated"));
      argx_str(x, &cft.options.extensions, &RSTR(".cft"));
    x=argx_init(arg_opt(arg), 'p', RSTR("partial"), RSTR("specify searching exact (+ case sensitive) or partially (+ ignores case)"));
      argx_bool(x, &cft.options.partial, 0);

    TRYC(arg_parse(arg, argc, argv, &exit_early));
    if(exit_early) goto clean;

    /* program start */
    TRYC(cft_arg(&cft, arg));
    TRYC(cft_init(&cft));

    /* read specified file */
    //TRYC(file_str_read(&cft.options.output, &cft.parse.content));
#if 1
    if(file_size(cft.options.output) != SIZE_MAX) {
        TRYC(file_exec(cft.options.output, &cft.parse.dirfiles, cft.options.recursive, cft_parse_file, &cft));
    }

    /* read all other specified files */
    if(!cft.options.modify || cft.options.merge) {
        for(size_t i = 0; i < vrstr_length(cft.options.inputs); ++i) {
            RStr input = *vrstr_get_at(&cft.options.inputs, i);
            Str pop = {0};
            if(!rstr_cmp(input, cft.options.output)) {
                /* TODO add some info here that skips two exact file paths... doesn't
                 * break anything if we DO, but helps to inform the user about what
                 * he's doing (absolutely not happened to me and I did NOT get confused
                 * by it) besides, expanding paths (.. / ~ / . etc) exists, sooo... */
                continue;
            }
            TRYC(file_exec(input, &cft.parse.dirfiles, cft.options.recursive, cft_parse_file, &cft));
            while(vstr_length(cft.parse.dirfiles)) {
                vstr_pop_back(&cft.parse.dirfiles, &pop);
                TRYC(file_exec(str_rstr(pop), &cft.parse.dirfiles, cft.options.recursive, cft_parse_file, &cft));
            }
        }
    }

    /* reformat */
    if(cft.options.modify || cft.options.merge) {
        if(!rstr_length(cft.options.output)) {
            arg_help_set(arg, cft.options.argx.output);
            arg_help(arg);
            THROW("no output provided");
            goto clean;
        }
        TRYC(cft_tags_add(&cft, &cft.options.rest, &cft.options.tags_add));
        //printff("RE [%.*s]", STR_F(&arg.parsed.tags_re));
        TRYC(cft_tags_re(&cft, &cft.options.rest, &cft.options.tags_re));
        str_clear(&cft.parse.content);
        //TRYC(cft_fmt(&cft, &cft.parse.content));
        //printf("%.*s", STR_F(&cft.parse.content));
        TRYC(file_str_write(cft.options.output, &cft.parse.content));
        goto clean;
    }

    /* query files */
    if(cft.options.query) {
        //printff("any [%.*s]", STR_F(arg.parsed.find_any));
        //printff("and [%.*s]", RSTR_F(arg.parsed.find_and));
        //printff("not [%.*s]", STR_F(arg.parsed.find_not));
        TRYC(cft_find_fmt(&cft, &ostream, &cft.options.find_any, &cft.options.find_and, &cft.options.find_not));
        printf("%.*s", STR_F(ostream));
        goto clean;
    }

    /* print files */
    if(cft.options.list_files && cft.options.list_files > cft.options.list_tags) {
        //printff(">> files fmt");
        TRYC(cft_files_fmt(&cft, &ostream, &cft.options.rest));
        printf("%.*s", STR_F(ostream));
        goto clean;
    }

    /* print tags */
    if(cft.options.list_tags && cft.options.list_tags > cft.options.list_files) {
        //printff(">> tags fmt");
        TRYC(cft_tags_fmt(&cft, &ostream, &cft.options.rest));
        printf("%.*s", STR_F(ostream));
        goto clean;
    }
#endif

clean:
    fflush(stdout);
    str_free(&ostream);
    cft_free(&cft);
    arg_free(&arg);
    info_handle_abort();
    return err;
error:
    ERR_CLEAN;
        }

