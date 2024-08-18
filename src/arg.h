#ifndef ARG_H

#include <stdbool.h>

#include "err.h"
#include "str.h"
#include "vector.h"

struct Nexus;

/* specify {{{ */

typedef enum {
    SPECIFY_NONE,
    /* below */
    SPECIFY_OPTIONAL,
    SPECIFY_OPTION,
        SPECIFY_OPTION_Y,
        SPECIFY_OPTION_YES,
        SPECIFY_OPTION_TRUE,
        SPECIFY_OPTION_N,
        SPECIFY_OPTION_NO,
        SPECIFY_OPTION_FALSE,
        SPECIFY_OPTION_NORMAL,
        SPECIFY_OPTION_SEARCH_ALL,
        SPECIFY_OPTION_SEARCH_SUB,
        SPECIFY_OPTION_ICON,
        SPECIFY_OPTION_ADD,
        SPECIFY_OPTION_DELETE,
        SPECIFY_OPTION_LIST,
        SPECIFY_OPTION_CLEAR,
        SPECIFY_OPTION_SELECT,
        SPECIFY_OPTION_SORT_STRING,
        SPECIFY_OPTION_SORT_NUMBER,
        SPECIFY_OPTION_SORT_NONE, /* !! REMEMBER to match SPECIFY__OPTION_END below !!! */
    SPECIFY_NUMBER,
    SPECIFY_STRING,
    SPECIFY_STRINGS,
    SPECIFY_LIST,
    SPECIFY_BOOL,
    /* certain default values */
    SPECIFY_EXTENSION,
    SPECIFY_MAX_FILE_SIZE,
    /* above */
    SPECIFY__COUNT,
    /* miscellaneous */
    SPECIFY__OPTION_BEGIN = SPECIFY_OPTION_Y,
    SPECIFY__OPTION_END = SPECIFY_OPTION_SORT_NONE,
} SpecifyList;

typedef struct Specify {
    size_t len;
    SpecifyList *ids;
} Specify;

#define SPECIFY(...)  (Specify){ \
    .ids = (SpecifyList []){__VA_ARGS__}, \
    .len = sizeof((SpecifyList []){__VA_ARGS__}) / sizeof(SpecifyList)}

/* }}} */

/* arguments {{{ */

typedef enum {
    ARG_NONE,
    /* args below */
    ARG_HELP,
    ARG_VERSION,
    ARG_TAG,
    ARG_RETAG,
    ARG_UNTAG,
    ARG_COPY,
    //ARG_LINK,
    ARG_REMOVE,
    ARG_RECURSIVE,
    ARG_MOVE,
    ARG_ANY,
    ARG_AND,
    ARG_NOT,
    ARG_SUBSTRING_TAGS,
    ARG_LIST_TAGS,
    ARG_LIST_FILES,
    ARG_TITLE,
    ARG_DECORATE,
    ARG_OUTPUT,
    ARG_INPUT,
    ARG_MERGE,
    ARG_COMPACT,
    ARG_EXPAND_PATHS,
    ARG_EXTENSIONS,
    // ARG_DEPTSH // folder depth
    // ARG_TAGS_RENAME
    ARG_EXISTS, // show either ONLY existing files, or NOT EXISTING files, if specified! -> nah, make like find. --type ?!
    /* args above */
    ARG__COUNT
} ArgList;

typedef struct Arg {
    const char *name;
    Str unknown;
    bool exit_early;
    VrStr *add_to;
    struct {
        VrStr remains;
        VrStr inputs;
        Str extensions;
        SpecifyList tag;
        Str file;
        int list_tags;
        int list_files;
        bool merge;
        bool expand_paths;
        bool compact;
        bool title;
        bool recursive;
        SpecifyList decorate;
        Str tags_add;
        Str tags_re;
        Str tags_del;
        Str find_any;
        Str find_and;
        Str find_not;
        Str substring_tags;
    } parsed;
    struct {
        int tiny; /* tiny, because short is reserved */
        int main;
        int ext; /* abbr. for extended, because long is reserved  */
        int spec; /* specification tabs */
        int max;
    } tabs;
} Arg;

/* }}} */

#define LINK_GITHUB "https://github.com/rphii/c-file-tagger"

#define ERR_ARG_PARSE "failed parsing arguments"
ErrDecl arg_parse(Arg *arg, size_t argc, const char **argv);

const char *arg_str(ArgList id);
const char *specify_str(SpecifyList id);

void arg_help(Arg *arg);
void arg_free(Arg *arg);

#define ARG_H
#endif

