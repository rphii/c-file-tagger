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
    struct ArgXGroup *o = 0;
    struct ArgX *x = 0;
    struct Arg *arg = arg_new();
    arg_init(arg, str_l(argv[0]), str("tag managing application"), str(""));
    arg_init_rest(arg, str("files|tags"), &cft.options.rest);
    arg_init_fmt(arg);
    o=argx_group(arg, str("Options"));
    argx_builtin_opt_help(o);
    argx_builtin_opt_source(o, str("/etc/cft/cft.conf"));
    argx_builtin_opt_source(o, str("$HOME/.config/rphiic/colors.conf"));
    argx_builtin_opt_source(o, str("$HOME/.config/cft/cft.conf"));
    argx_builtin_opt_source(o, str("$XDG_CONFIG_HOME/cft/cft.conf"));
    x=argx_init(o, 0 , str("version"), str("display the version"));
    x=argx_init(o, 't', str("tag"), str("tag files"));
      argx_str(x, &cft.options.tags_add, 0);
    x=argx_init(o, 0 , str("retag"), str("TBD rename files"));
      argx_str(x, &cft.options.tags_re, 0);
    x=argx_init(o, 'u', str("untag"), str("TBD untag files"));
      argx_str(x, &cft.options.tags_del, 0);
    x=argx_init(o, 'r', str("recursive"), str("recursively search subdirectories"));
      argx_bool(x, &cft.options.recursive, 0);
    x=argx_init(o, 'O', str("any"), str("list files with any tags"));
      argx_str(x, &cft.options.find_any, 0);
      argx_func(x, 0, set_do_query, &cft, false);
    x=argx_init(o, 'A', str("and"), str("list files having multiple tags"));
      argx_str(x, &cft.options.find_and, 0);
      argx_func(x, 0, set_do_query, &cft, false);
    x=argx_init(o, 'N', str("not"), str("list files not having tags"));
      argx_str(x, &cft.options.find_not, 0);
      argx_func(x, 0, set_do_query, &cft, false);
    x=argx_init(o, 'l', str("list-tags"), str("list all tags"));
      argx_func(x, 0, increment_list_tags, &cft, false);
    x=argx_init(o, 'L', str("list-files"), str("list all files"));
      argx_func(x, 0, increment_list_files, &cft, false);
    x=argx_init(o, 'T', str("title"), str("show title in output"));
      argx_bool(x, &cft.options.title, 0);
    x=argx_init(o, 'd', str("decorate"), str("specify decoration"));
      argx_bool(x, &cft.options.decorate, 0);
    x=argx_init(o, 'i', str("input"), str("specify additional input files"));
      argx_vstr(x, &cft.options.inputs, 0);
    x=argx_init(o, 'o', str("output"), str("specify output file"));
      argx_str(x, &cft.options.output, 0);
      cft.options.argx.output = x;
    x=argx_init(o, 'e', str("expand-paths"), str("expand paths"));
      argx_bool(x, &cft.options.expand_paths, 0);
    x=argx_init(o, 'x', str("extensions"), str("specify extensions, comma seperated"));
      argx_str(x, &cft.options.extensions, &STR(".cft"));
    x=argx_init(o, 'p', str("partial"), str("specify searching exact (+ case sensitive) or partially (+ ignores case)"));
      argx_bool(x, &cft.options.partial, 0);

    o=argx_group(arg, str("Environment Variables"));
    argx_builtin_env_compgen(o);

    o=argx_group(arg, str("Color Adjustments"));
    argx_builtin_opt_rice(o);

    TRYC(arg_parse(arg, argc, argv, &exit_early));
    if(exit_early) goto clean;

    /* program start */
    TRYC(cft_arg(&cft, arg));
    TRYC(cft_init(&cft));

    /* read specified file */
    //TRYC(file_str_read(&cft.options.output, &cft.parse.content));
#if 1
    if(file_size(cft.options.output) != SIZE_MAX) {
        TRYC(file_exec(cft.options.output, &cft.parse.dirfiles, cft.options.recursive, true, cft_parse_file, &cft));
    }

    /* read all other specified files */
    if(!cft.options.modify || cft.options.merge) {
        for(size_t i = 0; i < array_len(cft.options.inputs); ++i) {
            Str input = array_at(cft.options.inputs, i);
            Str pop = {0};
            if(!str_cmp(input, cft.options.output)) {
                /* TODO add some info here that skips two exact file paths... doesn't
                 * break anything if we DO, but helps to inform the user about what
                 * he's doing (absolutely not happened to me and I did NOT get confused
                 * by it) besides, expanding paths (.. / ~ / . etc) exists, sooo... */
                continue;
            }
            TRYC(file_exec(input, &cft.parse.dirfiles, cft.options.recursive, true, cft_parse_file, &cft));
            while(array_len(cft.parse.dirfiles)) {
                Str pop = array_pop(cft.parse.dirfiles);
                TRYC(file_exec(pop, &cft.parse.dirfiles, cft.options.recursive, true, cft_parse_file, &cft));
                str_free(&pop);
            }
        }
    }

    /* reformat */
    if(cft.options.modify || cft.options.merge) {
        if(!str_len_raw(cft.options.output)) {
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

