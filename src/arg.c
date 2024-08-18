#include <ctype.h>
#include <errno.h>

#include "arg.h"
#include "str.h"
#include "vector.h"

#include <pwd.h>

/* arguments */

static const char *static_arg[][2] = {
    [ARG_NONE] = {0, 0},
    [ARG_HELP] = {"h", "--help"},
    [ARG_VERSION] = {0, "--version"},
    [ARG_TAG] = {"t", "--tag"},
    [ARG_RETAG] = {0, "--retag"},
    [ARG_UNTAG] = {"u", "--untag"},
    [ARG_COPY] = {"y", "--copy"},
    //[ARG_LINK] = {"-l", "--link"},
    [ARG_REMOVE] = {0, "--remove"},
    [ARG_RECURSIVE] = {"r", "--recursive"},
    [ARG_MOVE] = {"m", "--move"},
    [ARG_ANY] = {"O", "--any"},
    [ARG_AND] = {"A", "--and"},
    [ARG_NOT] = {"N", "--not"},
    [ARG_SUBSTRING_TAGS] = {"s", "--substring-tags"},
    [ARG_LIST_TAGS] = {"l", "--list-tags"},
    [ARG_LIST_FILES] = {"L", "--list-files"},
    [ARG_EXISTS] = {0, "--exists"},
    [ARG_OUTPUT] = {"o", "--output"},
    [ARG_TITLE] = {"T", "--title"},
    [ARG_DECORATE] = {"d", "--decorate"},
    [ARG_INPUT] = {"i", "--input"},
    [ARG_MERGE] = {0, "--merge"},
    [ARG_COMPACT] = {"c", "--compact"},
    [ARG_EXPAND_PATHS] = {"e", "--expand"},
    [ARG_EXTENSIONS] = {"x", "--extensions"},
};

const char *arg_str(ArgList id)
{
    return static_arg[id][1];
}

static const char *static_desc[] = {
    [ARG_NONE] = 0,
    [ARG_HELP] = "print this help",
    [ARG_VERSION] = "display the version",
    [ARG_TAG] = "tag files",
    [ARG_RETAG] = "TBD rename tags",
    [ARG_UNTAG] = "TBD untag files",
    [ARG_COPY] = "TBD copy tags",
    //[ARG_LINK] = "TBD link tags",
    [ARG_REMOVE] = "TBD remove tags",
    [ARG_RECURSIVE] = "recursively search subdirectories",
    [ARG_MOVE] = "TBD move tags",
    [ARG_ANY] = "list files with any tags",
    [ARG_AND] = "list files having multiple tags",
    [ARG_NOT] = "list files not having tags",
    [ARG_SUBSTRING_TAGS] = "list tags where substrings match",
    [ARG_LIST_TAGS] = "list all tags",
    [ARG_LIST_FILES] = "list all files",
    [ARG_EXISTS] = "TBD show either only existing or not existing files, if specified",
    [ARG_OUTPUT] = "specify main file to be parsed",
    [ARG_TITLE] = "show title in output",
    [ARG_DECORATE] = "specify decoration",
    [ARG_INPUT] = "specify additional input files",
    [ARG_MERGE] = "merge all input files into the main file",
    [ARG_COMPACT] = "more compact output",
    [ARG_EXPAND_PATHS] = "treat tags as actual files and expand the paths properly",
    [ARG_EXTENSIONS] = "specify extension(s), comma separated list",
};

static const char static_version[] = ""
    #include "cft.version"
    "";

/* specify */

static const Specify static_specify[ARG__COUNT] = {
    [ARG_TAG] = SPECIFY(SPECIFY_LIST),
    [ARG_RETAG] = SPECIFY(SPECIFY_LIST),
    [ARG_UNTAG] = SPECIFY(SPECIFY_LIST),
    [ARG_COPY] = SPECIFY(SPECIFY_LIST),
    //[ARG_LINK] = SPECIFY(SPECIFY_LIST),
    [ARG_REMOVE] = {0},
    [ARG_MOVE] = SPECIFY(SPECIFY_STRING),
    [ARG_ANY] = SPECIFY(SPECIFY_LIST),
    [ARG_AND] = SPECIFY(SPECIFY_LIST),
    [ARG_NOT] = SPECIFY(SPECIFY_LIST),
    [ARG_SUBSTRING_TAGS] = SPECIFY(SPECIFY_LIST),
    [ARG_LIST_TAGS] = {0},
    [ARG_LIST_FILES] = {0},
    [ARG_OUTPUT] = SPECIFY(SPECIFY_STRING),
    [ARG_DECORATE] = SPECIFY(SPECIFY_OPTION, SPECIFY_OPTION_NO, SPECIFY_OPTION_N, SPECIFY_OPTION_FALSE, SPECIFY_OPTION_YES, SPECIFY_OPTION_TRUE, SPECIFY_OPTION_Y),
    [ARG_INPUT] = SPECIFY(SPECIFY_STRINGS),
    [ARG_MERGE] = {0},
    [ARG_EXPAND_PATHS] = {0},
    [ARG_EXTENSIONS] = SPECIFY(SPECIFY_LIST),
};

static const char *static_specify_str[] = {
    [SPECIFY_NONE] = "none",
    [SPECIFY_OPTIONAL] = "OPTIONAL",
    [SPECIFY_OPTION] = "OPTION",
        [SPECIFY_OPTION_NORMAL] = "normal",
        [SPECIFY_OPTION_SEARCH_ALL] = "search-all",
        [SPECIFY_OPTION_SEARCH_SUB] = "search-sub",
        [SPECIFY_OPTION_ICON] = "icon",
        [SPECIFY_OPTION_TRUE] = "true",
        [SPECIFY_OPTION_Y] = "y",
        [SPECIFY_OPTION_YES] = "yes",
        [SPECIFY_OPTION_FALSE] = "false",
        [SPECIFY_OPTION_N] = "n",
        [SPECIFY_OPTION_NO] = "no",
        [SPECIFY_OPTION_ADD] = "add",
        [SPECIFY_OPTION_DELETE] = "delete",
        [SPECIFY_OPTION_LIST] = "list",
        [SPECIFY_OPTION_CLEAR] = "clear",
        [SPECIFY_OPTION_SELECT] = "select",
    [SPECIFY_NUMBER] = "NUMBER",
    [SPECIFY_STRING] = "STRING",
    [SPECIFY_STRINGS] = "STRINGS",
    [SPECIFY_LIST] = "LIST",
    [SPECIFY_BOOL] = "< y | n >",
    /* certain default values */
    [SPECIFY_EXTENSION] = ".cft",
    [SPECIFY_MAX_FILE_SIZE] = "65536",
};

const char *specify_str(SpecifyList id)
{
    return static_specify_str[id];
}


int print_line(int max, int current, int tabs, Str *str)
{
    if(!str) return 0;
    int result = 0;
    int printed = 0;
    int length = 0;
    char *until = 0;
    for(size_t i = 0; i < str_length(str); i += (size_t)length) {
        while(isspace((int)str->s[i])) {
            i++;
        }
        if(i >= str_length(str)) break;
        if(!until && i) {
            printf("\n");
        }
        char *s = &str->s[i];
        until = strchr(s, '\n');
        length = until ? (int)(until + 1 - s) : (int)strlen(s);
        if(length + current > max) length = max - current;
        printed = printf("%*s%.*s", tabs, "", length, s);
        tabs = current;
        result += printed;
    }
    return result;
}

#define ERR_ARG_SPECIFIC_OPTIONAL "failed specifying optional argument"
ErrDeclStatic arg_static_specific_optional(Arg *arg, ArgList id, const char *specify)
{
    ASSERT(arg, ERR_NULL_ARG);
    switch(id) {
        default: THROW("id (%u) doesn't have a corresponding specific optional", id);
    }
    return 0;
error:
    return -1;
}

static void arg_static_print_version(Arg *arg)
{
    ASSERT(arg, ERR_NULL_ARG);
    if(strlen(static_version)) {
        printf("%s version %s-%s\n", arg->name, static_version, PLATFORM_NAME);
    } else {
        printf(F("failed ", FG_RD) "(no version present)\n");
    }
}

#define ERR_ARG_EXECUTE "failed executing argument"
ErrDeclStatic arg_static_execute(Arg *arg, ArgList id, Str *argY)
{
    ASSERT_ARG(arg);
    ASSERT_ARG(argY);
    if(id >= ARG__COUNT) THROW("incorrect id (%u)", id);
    void *to_verify = 0;
    switch(id) {
        case ARG_HELP: {
            arg_help(arg);
            arg->exit_early = true;
        } break;
        case ARG_VERSION: {
            arg_static_print_version(arg);
            arg->exit_early = true;
        } break;
        case ARG_EXPAND_PATHS: {
            arg->parsed.expand_paths = true;
        } break;
        case ARG_COMPACT: {
            arg->parsed.compact = true;
        } break;
        case ARG_TITLE: {
            arg->parsed.title = true;
        } break;
        case ARG_RECURSIVE: {
            arg->parsed.recursive = true;
        } break;
        case ARG_LIST_TAGS: {
            if(arg->parsed.list_files) ++arg->parsed.list_files;
            arg->parsed.list_tags = 1;
        } break;
        case ARG_LIST_FILES: {
            if(arg->parsed.list_tags) ++arg->parsed.list_tags;
            arg->parsed.list_files = 1;
        } break;
        case ARG_MERGE: { arg->parsed.merge = true; } break;
        case ARG_DECORATE: { to_verify = &arg->parsed.decorate; } break;
        case ARG_ANY: { to_verify = &arg->parsed.find_any; } break;
        case ARG_AND: { to_verify = &arg->parsed.find_and; } break;
        case ARG_NOT: { to_verify = &arg->parsed.find_not; } break;
        case ARG_OUTPUT: { to_verify = &arg->parsed.file; } break;
        case ARG_TAG: { to_verify = &arg->parsed.tags_add; } break;
        case ARG_UNTAG: { to_verify = &arg->parsed.tags_del; } break;
        case ARG_RETAG: { to_verify = &arg->parsed.tags_re; } break;
        case ARG_INPUT: { to_verify = &arg->parsed.inputs; } break;
        case ARG_SUBSTRING_TAGS: { to_verify = &arg->parsed.substring_tags; } break;
        case ARG_EXTENSIONS: { to_verify = &arg->parsed.extensions; } break;
        default: THROW(ERR_UNHANDLED_ID" (%d)", id);
    }
    if(to_verify) {
        Specify spec = static_specify[id];
        if(spec.len) {
            SpecifyList id_spec = spec.ids[0]; //*(SpecifyList *)to_verify; //spec.ids[0];
            if(id_spec <= SPECIFY__OPTION_END && id_spec >= SPECIFY__OPTION_BEGIN) {
                id_spec = SPECIFY_NONE;
            }
            switch(id_spec) {
                case SPECIFY_OPTION: {
                    id_spec = *(SpecifyList *)to_verify;
                    if(id_spec <= SPECIFY__OPTION_END && id_spec >= SPECIFY__OPTION_BEGIN) {
                        break;
                    }
                    arg->exit_early = true;
                    /* list options available */
                    const Specify *spec2 = &static_specify[id];
                    size_t n = 0;
                    for(size_t i = 1; i < spec2->len; ++i) {
                        if(spec2->ids[i]) ++n;
                    }
                    if(str_length(argY)) {
                        printf("%*s" F("%s", BOLD) " (one of %zu below; invalid provided: '" F("%.*s", BOLD) "')\n", arg->tabs.tiny, "", static_arg[id][1], n, STR_F(argY));
                    } else {
                        printf("%*s" F("%s", BOLD) " (one of %zu below; none provided)\n", arg->tabs.tiny, "", static_arg[id][1], n);
                    }
                    //bool first = true;
                    for(size_t i = 1; i < spec2->len; i++) {
                        SpecifyList h = spec2->ids[i];
                        if(!h) continue;
                        //first = false;
                        printf("%*s %s%s\n", arg->tabs.main, "", static_specify_str[h], i == 1 ? F(" (default)", IT) : "");
                    }
                    THROW("wrong or no option specified");
                } break;
                case SPECIFY_LIST:
                case SPECIFY_STRING: {
                    if(str_length((Str *)to_verify)) break;
                    arg->exit_early = true;
                    printf("%*s" F("%s %s", BOLD) " is empty\n", arg->tabs.tiny, "", arg_str(id), specify_str(id_spec));
                    THROW("no string specified");
                } break;
                case SPECIFY_STRINGS: {
                    if(vrstr_length((VrStr *)to_verify)) break;
                    arg->exit_early = true;
                    printf("%*s" F("%s %s", BOLD) " is empty\n", arg->tabs.tiny, "", arg_str(id), specify_str(id_spec));
                    THROW("no string specified");
                } break;
                case SPECIFY_NONE: break;
                default: THROW(ERR_UNHANDLED_ID" (%d) '%s' for %s", id_spec, specify_str(id_spec), arg_str(id));
            }
        }
    }
    return 0;
error:
    return -1;
}

#define ERR_ARG_ADD_TO_UNKNOWN "failed to add to unknown"
ErrDeclStatic arg_static_add_to_unknown(Arg *arg, Str *s)
{
    ASSERT(arg, ERR_NULL_ARG);
    ASSERT(s, ERR_NULL_ARG);
    bool add_comma = (str_length(&arg->unknown) != 0);
    TRY(str_fmt(&arg->unknown, "%s%s%.*s", add_comma ? ", " : "", str_length(s) > 1 ? "" : "-", STR_F(s)), ERR_STR_FMT);
    return 0;
error:
    return -1;
}

#define ERR_ARG_PARSE_SPEC "failed parsing specific argument"
ErrDeclStatic static_arg_parse_spec(Arg *args, ArgList arg, Str *argY, Specify spec, bool *argY_consumed)
{
    ASSERT(args, ERR_NULL_ARG);
    ASSERT(argY, ERR_NULL_ARG);
    if(!spec.len) return 0;
    SpecifyList id0 = spec.ids[0];
    void *to_set = 0;
    switch(arg) {
        case ARG_ANY: { to_set = &args->parsed.find_any; } break;
        case ARG_AND: { to_set = &args->parsed.find_and; } break;
        case ARG_NOT: { to_set = &args->parsed.find_not; } break;
        case ARG_DECORATE: { to_set = &args->parsed.decorate; } break;
        case ARG_TAG: { to_set = &args->parsed.tags_add; } break;
        case ARG_UNTAG: { to_set = &args->parsed.tags_del; } break;
        case ARG_RETAG: { to_set = &args->parsed.tags_re; } break;
        case ARG_OUTPUT: { to_set = &args->parsed.file; } break;
        case ARG_INPUT: { to_set = &args->parsed.inputs; } break;
        case ARG_SUBSTRING_TAGS: { to_set = &args->parsed.substring_tags; } break;
        case ARG_EXTENSIONS: { to_set = &args->parsed.extensions; } break;
        default: THROW("unhandled arg id (%u)", arg);
    }
    switch(id0) {
        case SPECIFY_OPTION: {
            SpecifyList *id_set = (SpecifyList *)to_set;
            *id_set = SPECIFY_OPTION;
            *argY_consumed = true;
            for(size_t k = 1; k < spec.len; ++k) {
                SpecifyList id = spec.ids[k];
                if(!id) continue;
                Str specs = STR_L((char *)static_specify_str[id]);
                //printff("  %zu:%.*s == %zu:%.*s\n", specs.last, STR_F(&specs), argY->last, STR_F(argY));
                if(str_cmp(&specs, argY)) continue;
                *id_set = id;
                //printf("  SELECTED!!!\n");
                break;
            }
        } break;
        case SPECIFY_STRING: {
            if(!str_length(argY)) THROW("no string specified"); // TODO:do I need this??
            *argY_consumed = true;
            if(str_length((Str *)to_set)) {
                printf("%*s" F("%s %.*s", BOLD) " can't specify more than one string\n", args->tabs.tiny, "", arg_str(arg), STR_F(argY));
                args->exit_early = true;
                return 0;
            }
            str_clear((Str *)to_set);
            TRYC(str_fmt((Str *)to_set, "%.*s", STR_F(argY)));
        } break;
        case SPECIFY_LIST: { /* TODO: can I really just memcpy? why did I fmt SPECIFY_STRING?? is that because if I rebuild? */
            *argY_consumed = true;
            memcpy((Str *)to_set, argY, sizeof(*argY));
        } break;
#if 0
        case SPECIFY_LIST: {
            Str split = {0};
            while(split = str_splice(argY, &split, ','), str_length(&split)) {
                TRY(vrstr_push_back(&args->extensions, &split), ERR_VEC_PUSH_BACK);
            }
            printff("extensions loaded: %zu", vrstr_length(&args->extensions));
        } break;
#endif
        case SPECIFY_NUMBER: {
            if(!str_length(argY)) THROW("no number specified");
            *argY_consumed = true;
            errno = 0;
            char *endptr = 0;
            char *begin = str_iter_begin(argY);
            size_t val = 0;
            if(begin) {
                val = (size_t)strtoll(begin, &endptr, 0);
            }
            if(*endptr) THROW("could not convert to number: %s (because of: %s)", begin, endptr);
            if(errno) THROW("strtoll conversion error");
            *(size_t *)to_set = val;
        } break;
        case SPECIFY_STRINGS: {
            if(!str_length(argY)) THROW(F("%s %s", BOLD) " is missing", arg_str(arg), specify_str(SPECIFY_STRING));
            *argY_consumed = true;
            //str_clear((Str *)to_set);
            TRY(vrstr_push_back((VrStr *)to_set, argY), ERR_VEC_PUSH_BACK);
            //TRYC(str_fmt((Str *)to_set, "%.*s", STR_F(argY)));
        } break;
        default: THROW("unhandled id0! (%u)", id0);
    }
    return 0;
error:
    return -1;
}

int arg_parse(Arg *args, size_t argc, const char **argv) /* {{{ */
{
    ASSERT(args, ERR_NULL_ARG);
    ASSERT(argv, ERR_NULL_ARG);
    /* set up */
    size_t err_index = 0;
    size_t err_arg_many = 0;
    ArgList err_arg = 0;
    args->name = argv[0];
    args->tabs.tiny = 2;
    args->tabs.main = 7;
    args->tabs.ext = 34;
    args->tabs.spec = args->tabs.ext + 2;
    args->tabs.max = 100;
    //VrStr many_cmp = {0};
    /* TODO add this into a function */
    //arg_help(args);
    /* actually parse */
    for(size_t i = 1; i < argc; ++i) {
        err_index = i;
        /* current argument */
        Str arg = STR_L((char *)argv[i]);
        Str argX = {0};
        /* determine if it's short arguments or full text */
        size_t argY_i = 0;
        size_t arg_many = 0;
        if(str_get_front(&arg) != '-') {
            TRY(vrstr_push_back(&args->parsed.remains, &arg), ERR_VEC_PUSH_BACK);
            continue;
        }
        bool arg_opt = false; /* assume arg is one dash */
        if(!str_cmp(&STR_LL(str_iter_begin(&arg), str_length(&arg) > 2 ? 2 : str_length(&arg)), &STR("--"))) arg_opt = true; /* arg is two dashes */
        if(!arg_opt) ++arg_many;
        /* get a possible value provided with = */
        size_t posY = str_ch(&arg, '=', 0);
        Str argYY = STR("");
        bool argYY_available = false;
        if(posY < str_length(&arg)) {
            argYY = STR_L(str_iter_at(&arg, posY + 1));
            argYY_available = true;
        }
        //printff("[%zu] argYY [%.*s] available ? %s", i, STR_F(&argYY), argYY_available ? "true" : "false");
        /* find matching argument from our list */
        for(;;) {
            ArgList j = 0;
            err_arg_many = arg_many;
            for(j = 0; j < ARG__COUNT; ++j) {
                /* prepare for comparison to find current argument */
                Str cmp = STR_L((char *)static_arg[j][arg_opt]);
                if(!str_length(&cmp)) continue; /* arg to cmp is not even an argument */
                argX = STR_LL(str_iter_at(&arg, arg_many), 1); /* assume short arguments -> length 1 */
                if(arg_opt) argX.last = argX.first + posY; /* if long arguments, fix length */
                if(str_cmp(&argX, &cmp)) continue; /* arg to cmp to is not equal */
                /* found our current argument! */
                err_arg = j;
                Specify spec = static_specify[j];
                /* get a possible arg value */
                bool argY_consumed = false;
                Str argY = STR("");
                if(argYY_available) argY = argYY;
                else if(i + argY_i + 1 < argc) argY = STR_L((char *)argv[i + argY_i + 1]);
                /* parse & process */
                //printff("[%s] argY = [%zu]->[%.*s]", arg_str(j), i+argY_i+1, STR_F(&argY));
                TRY(static_arg_parse_spec(args, j, &argY, spec, &argY_consumed), ERR_ARG_PARSE_SPEC);
                TRY(arg_static_execute(args, j, &argY), ERR_ARG_EXECUTE);
                //printff("argY consumed? %s", argY_consumed ? "true" : "false");
                if(argY_consumed) {
                    if(argYY_available) {
                        argYY_available = false;
                    } else {
                        ++argY_i;
                    }
                }
                break;
                //printff("[i+argY_i+1] = [%zu+%zu+1]", i, argY_i);
                //getchar();
            }
            /* confirm for a valid option */
            if(j == ARG__COUNT) {
                if(arg_opt) {
                    TRY(arg_static_add_to_unknown(args, &arg), ERR_ARG_ADD_TO_UNKNOWN);
                } else {
                    TRY(arg_static_add_to_unknown(args, &argX), ERR_ARG_ADD_TO_UNKNOWN);
                }
            }
            /* is argYY used up? */
            if(argYY_available) {
                //TRY(arg_static_add_to_unknown(args, &argX), ERR_ARG_ADD_TO_UNKNOWN);
                THROW("argument does not take anything additional");
            }
            /* done, next */
            if(arg_opt) break;
            ++arg_many;
            if(arg_many >= posY) break;
            err_arg = 0;
        }
        i += argY_i;
    }
#if 1
    if(str_length(&args->unknown)) {
        THROW("unknown arguments"); //: %.*s", STR_F(&args->unknown));
    }
    /* post processing */
    if((args->parsed.list_tags || args->parsed.list_files) && !str_length(&args->parsed.find_and) && !str_length(&args->parsed.find_any) && !str_length(&args->parsed.find_not)) {
        if(!args->parsed.decorate) {
            args->parsed.decorate = SPECIFY_OPTION_YES;
        }
    }
    /* TODO add this into a function */
    /* default arguments */
    if(!str_length(&args->parsed.extensions)) {
        args->parsed.extensions = STR_L(static_specify_str[SPECIFY_EXTENSION]);
    }
    if(args->parsed.merge && !vrstr_length(&args->parsed.inputs)) { /* TODO: ... do I want this in arg.c or in cft.c ??? */
        THROW("no input files specified (" F("%s", BOLD) "), nothing to merge", arg_str(ARG_INPUT));
    }
#endif
    return 0;
error:
    if(str_length(&args->unknown)) {
        printf("%*sUnknown arguments: '%.*s'\n", args->tabs.tiny, "", STR_F(&args->unknown));
    }
    if(err_index) {
        if(err_arg && err_arg < ARG__COUNT) {
            if(err_arg_many) {
                printf("%*sFailed parsing argument: '" F("%s", BOLD) "' (from [%zu] of '" F("%s", BOLD) "')\n", args->tabs.tiny, "", arg_str(err_arg), err_arg_many - 1, argv[err_index] );
            } else {
                printf("%*sFailed parsing argument: '" F("%s", BOLD) "' (from '" F("%s", BOLD) "')\n", args->tabs.tiny, "", arg_str(err_arg), argv[err_index] );
            }
        }
    }
    return -1;
}; /* }}} */

void arg_help(Arg *arg) /* {{{ */
{
    ASSERT(arg, ERR_NULL_ARG);
    int err = 0;
    Str ts = {0};
    TRYC(str_fmt(&ts, F("%s:", BOLD) " tag managing application.", arg->name));
    print_line(arg->tabs.max, 0, 0, &ts);
    str_clear(&ts);
    printf("\n\n");
    print_line(arg->tabs.max, 0, 0, &STR("Usage:\n"));
    TRY(str_fmt(&ts, "%s [options]\n", arg->name), ERR_STR_FMT);
    print_line(arg->tabs.max, arg->tabs.main, arg->tabs.main, &ts);
    printf("\n");
    print_line(arg->tabs.max, 0, 0, &STR("Options:\n"));
    for(size_t i = 0; i < ARG__COUNT; i++) {
        int tabs_offs = 0;
        int tp = 0;
        if(!i) continue;
        const char *arg_short = static_arg[i][0];
        const char *arg_long = static_arg[i][1];
        if(arg_short) {
            str_clear(&ts);
            TRY(str_fmt(&ts, "-%s,", arg_short), ERR_STR_FMT);
            tp = print_line(arg->tabs.max, arg->tabs.tiny, arg->tabs.tiny, &ts);
            tabs_offs += tp;
        }
        if(arg_long) {
            str_clear(&ts);
            const char *explained = static_desc[i];
            const Specify *specify = &static_specify[i];
            TRY(str_fmt(&ts, "%s%s", arg_long, specify->len ? " " : ""), ERR_STR_FMT);
            tp = print_line(arg->tabs.max, arg->tabs.main, arg->tabs.main-tabs_offs, &ts);
            tabs_offs += tp;
            if(specify->len) {
                str_clear(&ts);
                const char *sgl = static_specify_str[specify->ids[0]];
                TRY(str_fmt(&ts, "%s", sgl ? sgl : F("!!! missing string representation !!!", FG_RD_B)), ERR_STR_FMT);
                tp = print_line(arg->tabs.max, arg->tabs.spec, 0, &ts);
                tabs_offs += tp;
            }
            if(!explained) {
                ABORT("!!! missing argument explanation !!!");
            } else {
                str_clear(&ts);
                TRY(str_fmt(&ts, "%s ", explained), ERR_STR_FMT);
                tp = print_line(arg->tabs.max, arg->tabs.ext, arg->tabs.ext-tabs_offs, &ts);
                tabs_offs += tp;
                if(specify->len > 1) {
                    printf("\n%*s", arg->tabs.ext+2, "");
                    str_clear(&ts);
                    bool first = true;
                    for(size_t j = 1; j < specify->len; j++) {
                        const char *spec = static_specify_str[specify->ids[j]];
                        if(!spec || !specify->ids[j]) continue;
                        TRY(str_fmt(&ts, "%s%s", first ? "" : ", ", spec), ERR_STR_FMT);
                        first = false;
                        if(j == 1) {
                            if(specify->ids[0] == SPECIFY_OPTION) {
                                TRYC(str_fmt(&ts, ""F(" (default)", IT)""));
                            } else if(specify->ids[0] == SPECIFY_OPTIONAL) {
                                TRYC(str_fmt(&ts, ""F(" (optional)", IT)""));
                            } else if(specify->ids[0] == SPECIFY_LIST) {
                                TRYC(str_fmt(&ts, ""F(" (default)", IT)""));
                            } else if(specify->ids[0] == SPECIFY_NUMBER) {
                                TRYC(str_fmt(&ts, ""F(" (default)", IT)""));
                            } else {
                                ABORT("!!! missing behavior hint !!!");
                            }
                        }
                    }
                    tp = print_line(arg->tabs.max, arg->tabs.spec, 0, &ts);
                }
            }
        }
        switch(i) {
            /* extra info on specific default arguments */
            case ARG_DECORATE: {
                str_clear(&ts);
                TRYC(str_fmt(&ts, "\n" F("(if only %s or %s, default to %s)", IT), arg_str(ARG_LIST_TAGS), arg_str(ARG_LIST_FILES), specify_str(SPECIFY_OPTION_YES)));
                tp = print_line(arg->tabs.max, 0, arg->tabs.spec, &ts);
            } break;
            case ARG_EXTENSIONS: {
                str_clear(&ts);
                TRYC(str_fmt(&ts, "\n%s " F("(default)", IT), static_specify_str[SPECIFY_EXTENSION]));
                tp = print_line(arg->tabs.max, 0, arg->tabs.spec, &ts);
            } break;
            default: break;
        }
        printf("\n");
    }
    printf("\n");
    printf("GitHub: %s\n", LINK_GITHUB);
clean:
    str_free(&ts);
    return; (void)err;
error: ERR_CLEAN;
} /* }}} */

void arg_free(Arg *arg)
{
    str_free(&arg->unknown);
    //str_free(&arg->parsed.extensions);
    str_free(&arg->parsed.file);
    vrstr_free(&arg->parsed.remains);
    vrstr_free(&arg->parsed.inputs);
}


