#ifndef CFT_H

#include "err.h"
#include "vector.h"
#include "lookup.h"
#include <rlarg.h>

typedef struct CftBase {
    TStr strings;
    TTPStr file_tags;
    TTPStr tag_files;
} CftBase;

typedef struct Cft {
    CftBase base;
    So comments;
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
        bool partial;
        int list_tags;
        int list_files;
        So extensions;
        So output;
        VSo inputs;
        VSo rest;

        So tags_re;
        So tags_add;
        So tags_del;

        So find_and;
        So find_any;
        So find_not;
        //So *find_and;
        //So *find_any;
        //So *find_not;
        //So *tags_add;
        //So *tags_del;
        struct {
            struct ArgX *output;
        } argx;
    } options;
    struct {
        VSo dirfiles;
        So extension;
        So content;
    } parse;
    struct {
        So homedir;
        So current_dir;
    } misc;
} Cft;

#define ERR_cft_init(...) "failed initializing c-file-tagger"
ErrDecl cft_init(Cft *cft);

#define ERR_cft_arg(...) "failed passing arguments to c-file-tagger"
ErrDecl cft_arg(Cft *cft, struct Arg *arg);

#define ERR_cft_add(x, filename, tag) "failed adding tag '%.*s' to file '%.*s' in c-file-tagger", SO_F(filename), SO_F(tag)
ErrDecl cft_add(Cft *cft, const So filename, const So tag);

#define ERR_cft_retag(x, xy, from, to) "failed renaming '%.*s' to '%.*s'", SO_F(from), SO_F(to)
ErrDecl cft_retag(Cft *cft, const So *filename, const So *from, const So *to);

#define ERR_cft_del(x, filename, tag) "failed removing tag '%.*s' from file '%.*s' in c-file-tagger", SO_F(filename), SO_F(tag)
ErrDecl cft_del(Cft *cft, const So *filename, const So *tag);

#define ERR_cft_find_by_tag(cft, x, tag, ...) "failed finding tag: '%.*s'", SO_F(tag)
//ErrDecl cft_find_by_tag(Cft *cft, TrStr **foundr, const So *tag, bool create_if_nonexist);

#define ERR_cft_find_by_filename(cft, x, filename, ...) "failed finding filename: '%.*s'", SO_F(filename)
//ErrDecl cft_find_by_filename(Cft *cft, TrStr **found, const So *filename, bool create_if_nonexist);

#define ERR_cft_parse(...) "failed parsing"
ErrDecl cft_parse(Cft *cft, const So input, const So *str);

#define ERR_cft_parse_dir(filename, ...) "failed parsing '%.*s'", SO_F(filename)
ErrDecl cft_parse_file(So filename, void *cft_void);

#define ERR_cft_parse_folder(filename, ...) "failed parsing '%.*s'", SO_F(filename)
ErrDecl cft_parse_dir(So filename, void *cft_void);

#define ERR_cft_del_duplicate_folders(...) "failed removing duplicate folders (for export)"
ErrDecl cft_del_duplicate_folders(Cft *cft);

#define ERR_cft_fmt(...) "failed formatting"
ErrDecl cft_fmt(Cft *cft, So *str);

#define ERR_cft_find_any(x, y, find, ...) "failed finding 'any' for tags '%.*s'", SO_F(*find)
//ErrDecl cft_find_any(Cft *cft, TrrTag *found, So *find);

#define ERR_cft_find_and(x, y, find, ...) "failed finding 'and' for tags '%.*s'", SO_F(*find)
//ErrDecl cft_find_and(Cft *cft, TrrTag *found, So *find, bool first_query);

#define ERR_cft_find_not(x, y, find, ...) "failed finding 'not' for tags '%.*s'", SO_F(*find)
//ErrDecl cft_find_not(Cft *cft, TrrTag *found, So *find, bool first_query);

#define ERR_cft_tags_add(...) "failed adding tags"
ErrDecl cft_tags_add(Cft *cft, VSo *files, So *tags);

#define ERR_cft_tags_re(...) "failed renaming tags"
ErrDecl cft_tags_re(Cft *cft, VSo *files, So *tags);

#define ERR_cft_fmt_substring_tags(...) "failed formatting substring tags"
ErrDecl cft_fmt_substring_tags(Cft *cft, So *ostream, So *tags);

#define ERR_cft_tags_fmt(...) "failed formatting tags"
ErrDecl cft_tags_fmt(Cft *cft, So *out, VSo *files);

#define ERR_cft_files_fmt(...) "failed formatting files"
ErrDecl cft_files_fmt(Cft *cft, So *out, VSo *files);

#define ERR_cft_find_fmt(...) "failed formatting findings"
ErrDecl cft_find_fmt(Cft *cft, So *out, So *find_any, So *find_and, So *find_not);

void cft_free(Cft *cft);

#define CFT_H
#endif

