//usr/bin/env tcc -DPROC_COUNT=4 $(ls *.c | grep -v main.c) -run "$0" "$@" ; exit $?

#include <stdio.h>

#include <rlso.h>
#include <rlarg.h>
#include "lookup.h"
#include "platform.h"
#include "info.h"
#include "cft.h"


#include <linux/limits.h>

//#include <ctype.h>

int increment_list_tags(struct Argx *argx, void *user, So so) {
    ASSERT_ARG(user);
    Cft *x = user;
    if(x->options.list_files) ++x->options.list_files;
    ++x->options.list_tags;
    return 0;
}

int increment_list_files(struct Argx *argx, void *user, So so) {
    ASSERT_ARG(user);
    Cft *x = user;
    if(x->options.list_tags) ++x->options.list_tags;
    ++x->options.list_files;
    return 0;
}

int set_do_query(struct Argx *argx, void *user, So so) {
    ASSERT_ARG(user);
    Cft *x = user;
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
    So ostream = {0};

    bool exit_early = false;
    struct Argx_Group *o = 0;
    struct Argx *x = 0;
    struct Arg_Config *arg_config = arg_config_new();
    arg_config_set_program(arg_config, so_l(argv[0]));
    arg_config_set_description(arg_config, so("tag managing application"));
    struct Arg *arg = arg_new(arg_config);

    x=argx_pos(arg, so("files|tags"), so(""));
      argx_type_rest(x, &cft.options.rest);

    o=argx_group(arg, so("options"));
    argx_builtin_opt_help(o, ARGX_BUILTIN_OPT_HELP);
    argx_builtin_opt_source(o, ARGX_BUILTIN_OPT_SOURCE, so("/etc/cft/cft.conf"));
    argx_builtin_opt_source(o, ARGX_BUILTIN_OPT_SOURCE, so("$HOME/.config/rphiic/colors.conf"));
    argx_builtin_opt_source(o, ARGX_BUILTIN_OPT_SOURCE, so("$HOME/.config/cft/cft.conf"));
    argx_builtin_opt_source(o, ARGX_BUILTIN_OPT_SOURCE, so("$XDG_CONFIG_HOME/cft/cft.conf"));
    x=argx_opt(o, 0 , so("version"), so("display the version"));
    x=argx_opt(o, 't', so("tag"), so("tag files"));
      argx_type_so(x, &cft.options.tags_add, 0);
    x=argx_opt(o, 0 , so("retag"), so("TBD rename files"));
      argx_type_so(x, &cft.options.tags_re, 0);
    x=argx_opt(o, 'u', so("untag"), so("TBD untag files"));
      argx_type_so(x, &cft.options.tags_del, 0);
    x=argx_opt(o, 'r', so("recursive"), so("recursively search subdirectories"));
      argx_type_bool(x, &cft.options.recursive, 0);
    x=argx_opt(o, 'O', so("any"), so("list files with any tags"));
      argx_type_so(x, &cft.options.find_any, 0);
      argx_callback(x, set_do_query, &cft, ARGX_PRIORITY_IMMEDIATELY);
    x=argx_opt(o, 'A', so("and"), so("list files having multiple tags"));
      argx_type_so(x, &cft.options.find_and, 0);
      argx_callback(x, set_do_query, &cft, ARGX_PRIORITY_IMMEDIATELY);
    x=argx_opt(o, 'N', so("not"), so("list files not having tags"));
      argx_type_so(x, &cft.options.find_not, 0);
      argx_callback(x, set_do_query, &cft, ARGX_PRIORITY_IMMEDIATELY);
    x=argx_opt(o, 'l', so("list-tags"), so("list all tags"));
      argx_callback(x, increment_list_tags, &cft, ARGX_PRIORITY_IMMEDIATELY);
    x=argx_opt(o, 'L', so("list-files"), so("list all files"));
      argx_callback(x, increment_list_files, &cft, ARGX_PRIORITY_IMMEDIATELY);
    x=argx_opt(o, 'T', so("title"), so("show title in output"));
      argx_type_bool(x, &cft.options.title, 0);
    x=argx_opt(o, 'd', so("decorate"), so("specify decoration"));
      argx_type_bool(x, &cft.options.decorate, 0);
    x=argx_opt(o, 'i', so("input"), so("specify additional input files"));
      argx_type_array_so(x, &cft.options.inputs, 0);
    x=argx_opt(o, 'o', so("output"), so("specify output file"));
      argx_type_so(x, &cft.options.output, 0);
      cft.options.argx.output = x;
    x=argx_opt(o, 'e', so("expand-paths"), so("expand paths"));
      argx_type_bool(x, &cft.options.expand_paths, 0);
    x=argx_opt(o, 'x', so("extensions"), so("specify extensions, comma seperated"));
      argx_type_so(x, &cft.options.extensions, &so(".cft"));
    x=argx_opt(o, 'p', so("partial"), so("specify searching exact (+ case sensitive) or partially (+ ignores case)"));
      argx_type_bool(x, &cft.options.partial, 0);

    argx_builtin_env_compgen(arg);
    argx_builtin_rice(arg);

    err = arg_parse(arg, argc, argv, &exit_early);
    if(err || exit_early) goto clean;

    /* program start */
    TRYC(cft_arg(&cft, arg));
    TRYC(cft_init(&cft));

    /* read specified file */
    //TRYC(file_str_read(&cft.options.output, &cft.parse.content));
#if 1
    size_t size = 0;
    if(!so_file_get_size(cft.options.output, &size)) {
        //TRYC(so_file_exec(cft.options.output, &cft.parse.dirfiles, cft.options.recursive, true, cft_parse_file, &cft));
        TRYC(so_file_exec(cft.options.output, true, cft.options.recursive, cft_parse_file, cft_parse_dir, &cft));
    }

    /* read all other specified files */
    if(!cft.options.modify || cft.options.merge) {
        for(size_t i = 0; i < array_len(cft.options.inputs); ++i) {
            So input = array_at(cft.options.inputs, i);
            So pop = {0};
            if(!so_cmp(input, cft.options.output)) {
                /* TODO add some info here that skips two exact file paths... doesn't
                 * break anything if we DO, but helps to inform the user about what
                 * he's doing (absolutely not happened to me and I did NOT get confused
                 * by it) besides, expanding paths (.. / ~ / . etc) exists, sooo... */
                continue;
            }
            //TRYC(file_exec(input, &cft.parse.dirfiles, cft.options.recursive, true, cft_parse_file, &cft));
            TRYC(so_file_exec(input, true, cft.options.recursive, cft_parse_file, cft_parse_dir, &cft));
            while(array_len(cft.parse.dirfiles)) {
                So pop = array_pop(cft.parse.dirfiles);
                //TRYC(so_file_exec(pop, &cft.parse.dirfiles, cft.options.recursive, true, cft_parse_file, &cft));
                TRYC(so_file_exec(pop, true, cft.options.recursive, cft_parse_file, cft_parse_dir, &cft));
                so_free(&pop);
            }
        }
    }

    /* reformat */
    if(cft.options.modify || cft.options.merge) {
        if(!so_len(cft.options.output)) {
            ABORT("TODO");
            //arg_help_set(arg, cft.options.argx.output);
            arg_help(arg);
            THROW("no output provided");
            goto clean;
        }
        TRYC(cft_tags_add(&cft, &cft.options.rest, &cft.options.tags_add));
        //printff("RE [%.*s]", SO_F(&arg.parsed.tags_re));
        TRYC(cft_tags_re(&cft, &cft.options.rest, &cft.options.tags_re));
        so_clear(&cft.parse.content);
        //TRYC(cft_fmt(&cft, &cft.parse.content));
        //printf("%.*s", SO_F(&cft.parse.content));
        TRYC(so_file_write(cft.options.output, cft.parse.content));
        goto clean;
    }

    /* query files */
    if(cft.options.query) {
        //printff("any [%.*s]", SO_F(arg.parsed.find_any));
        //printff("and [%.*s]", RSO_F(arg.parsed.find_and));
        //printff("not [%.*s]", SO_F(arg.parsed.find_not));
        TRYC(cft_find_fmt(&cft, &ostream, &cft.options.find_any, &cft.options.find_and, &cft.options.find_not));
        printf("%.*s", SO_F(ostream));
        goto clean;
    }

    /* print files */
    if(cft.options.list_files && cft.options.list_files > cft.options.list_tags) {
        //printff(">> files fmt");
        TRYC(cft_files_fmt(&cft, &ostream, &cft.options.rest));
        printf("%.*s", SO_F(ostream));
        goto clean;
    }

    /* print tags */
    if(cft.options.list_tags && cft.options.list_tags > cft.options.list_files) {
        //printff(">> tags fmt");
        TRYC(cft_tags_fmt(&cft, &ostream, &cft.options.rest));
        printf("%.*s", SO_F(ostream));
        goto clean;
    }
#endif

clean:
    fflush(stdout);
    so_free(&ostream);
    cft_free(&cft);
    arg_free(&arg);
    info_handle_abort();
    return err;
error:
    ERR_CLEAN;
        }

