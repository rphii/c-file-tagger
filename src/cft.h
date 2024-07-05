#ifndef CFT_H

#include "err.h"
#include "vector.h"
#include "lookup.h"
#include "tag.h"

typedef struct Cft {
    TrTag tags;
    TrTagRef reverse;
    TrrTagRef all;
} Cft;

#define ERR_cft_init(...) "failed initializing c-file-tagger"
ErrDecl cft_init(Cft *cft);

#define ERR_cft_add(x, filename, tag) "failed adding tag '%.*s' to file '%.*s' in c-file-tagger", STR_F(filename), STR_F(tag)
ErrDecl cft_add(Cft *cft, const Str *filename, const Str *tag);

#define ERR_cft_find_by_tag(cft, x, tag, ...) "failed finding tag: '%.*s'", STR_F(tag)
ErrDecl cft_find_by_tag(Cft *cft, TagRef **foundr, const Str *tag, bool create_if_nonexist);

#define ERR_cft_find_by_filename(cft, x, filename, ...) "failed finding filename: '%.*s'", STR_F(filename)
ErrDecl cft_find_by_filename(Cft *cft, Tag **found, const Str *filename, bool create_if_nonexist);

#define ERR_cft_parse(...) "failed parsing"
ErrDecl cft_parse(Cft *cft, const Str *str);

#define ERR_cft_fmt(...) "failed formatting"
ErrDecl cft_fmt(Cft *cft, Str *str);

#define ERR_cft_find_any(x, y, find, ...) "failed finding 'any' for tags '%.*s'", STR_F(find)
ErrDecl cft_find_any(Cft *cft, TrrTag *found, Str *find);

#define ERR_cft_find_and(x, y, find, ...) "failed finding 'and' for tags '%.*s'", STR_F(find)
ErrDecl cft_find_and(Cft *cft, TrrTag *found, Str *find);

#define ERR_cft_find_not(x, y, find, ...) "failed finding 'not' for tags '%.*s'", STR_F(find)
ErrDecl cft_find_not(Cft *cft, TrrTag *found, Str *find);

#define ERR_cft_list_fmt(...) "failed formatting list"
ErrDecl cft_list_fmt(Cft *cft, Str *out);

#define ERR_cft_find_fmt(...) "failed formatting findings"
ErrDecl cft_find_fmt(Cft *cft, Str *out, Str *find_any, Str *find_and, Str *find_not, bool list_tags);

void cft_free(Cft *cft);

#define CFT_H
#endif

