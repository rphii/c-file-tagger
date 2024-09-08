#ifndef CFT_H

#include "err.h"
#include "vector.h"
#include "lookup.h"
#include "tag.h"
#include "arg.h"

typedef struct CftBase {
    TTrStr file_tags;
    TTrStr tag_files;
} CftBase;

typedef struct Cft {
    CftBase base;
    Str comments;
    //TrrTagRef all;
    struct {
        bool decorate;
        bool query;
        bool modify;
        bool merge;
        bool expand_paths;
        bool compact;
        bool title;
        bool recursive;
        int list_tags;
        int list_files;
        Str extensions;
        //Str *find_and;
        //Str *find_any;
        //Str *find_not;
        //Str *tags_add;
        //Str *tags_del;
    } options;
    struct {
        VStr dirfiles;
        Str extension;
        Str content;
    } parse;
    struct {
        Str homedir;
        Str current_dir;
    } misc;
} Cft;

#define ERR_cft_init(...) "failed initializing c-file-tagger"
ErrDecl cft_init(Cft *cft);

#define ERR_cft_arg(...) "failed passing arguments to c-file-tagger"
ErrDecl cft_arg(Cft *cft, Arg *arg);

#define ERR_cft_add(x, filename, tag) "failed adding tag '%.*s' to file '%.*s' in c-file-tagger", STR_F(filename), STR_F(tag)
ErrDecl cft_add(Cft *cft, const Str *filename, const Str *tag);

#define ERR_cft_retag(x, xy, from, to) "failed renaming '%.*s' to '%.*s'", STR_F(from), STR_F(to)
ErrDecl cft_retag(Cft *cft, const Str *filename, const Str *from, const Str *to);

#define ERR_cft_del(x, filename, tag) "failed removing tag '%.*s' from file '%.*s' in c-file-tagger", STR_F(filename), STR_F(tag)
ErrDecl cft_del(Cft *cft, const Str *filename, const Str *tag);

#define ERR_cft_find_by_tag(cft, x, tag, ...) "failed finding tag: '%.*s'", STR_F(tag)
//ErrDecl cft_find_by_tag(Cft *cft, TrStr **foundr, const Str *tag, bool create_if_nonexist);

#define ERR_cft_find_by_filename(cft, x, filename, ...) "failed finding filename: '%.*s'", STR_F(filename)
//ErrDecl cft_find_by_filename(Cft *cft, TrStr **found, const Str *filename, bool create_if_nonexist);

#define ERR_cft_parse(...) "failed parsing"
ErrDecl cft_parse(Cft *cft, const Str *input, const Str *str);

#define ERR_cft_parse_file(filename, ...) "failed parsing '%.*s'", STR_F(filename)
ErrDecl cft_parse_file(Str *filename, void *cft_void);

#define ERR_cft_del_duplicate_folders(...) "failed removing duplicate folders (for export)"
ErrDecl cft_del_duplicate_folders(Cft *cft);

#define ERR_cft_fmt(...) "failed formatting"
ErrDecl cft_fmt(Cft *cft, Str *str);

#define ERR_cft_find_any(x, y, find, ...) "failed finding 'any' for tags '%.*s'", STR_F(find)
//ErrDecl cft_find_any(Cft *cft, TrrTag *found, Str *find);

#define ERR_cft_find_and(x, y, find, ...) "failed finding 'and' for tags '%.*s'", STR_F(find)
//ErrDecl cft_find_and(Cft *cft, TrrTag *found, Str *find, bool first_query);

#define ERR_cft_find_not(x, y, find, ...) "failed finding 'not' for tags '%.*s'", STR_F(find)
//ErrDecl cft_find_not(Cft *cft, TrrTag *found, Str *find, bool first_query);

#define ERR_cft_tags_add(...) "failed adding tags"
ErrDecl cft_tags_add(Cft *cft, VrStr *files, Str *tags);

#define ERR_cft_tags_re(...) "failed renaming tags"
ErrDecl cft_tags_re(Cft *cft, VrStr *files, Str *tags);

#define ERR_cft_fmt_substring_tags(...) "failed formatting substring tags"
ErrDecl cft_fmt_substring_tags(Cft *cft, Str *ostream, Str *tags);

#define ERR_cft_tags_fmt(...) "failed formatting tags"
ErrDecl cft_tags_fmt(Cft *cft, Str *out, VrStr *files);

#define ERR_cft_files_fmt(...) "failed formatting files"
ErrDecl cft_files_fmt(Cft *cft, Str *out, VrStr *files);

#define ERR_cft_find_fmt(...) "failed formatting findings"
ErrDecl cft_find_fmt(Cft *cft, Str *out, Str *find_any, Str *find_and, Str *find_not);

void cft_free(Cft *cft);

#define CFT_H
#endif

