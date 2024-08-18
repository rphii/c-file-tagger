#ifndef STR_H

#define STR_DEFAULT_SIZE 32

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "err.h"

typedef uint8_t V3u8[3];

/* configuration, inclusion and de-configuration of vector */

#define VEC_SETTINGS_DEFAULT_SIZE STR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s
#include "vec.h"

VEC_INCLUDE(Str, str, char, BY_VAL);

#undef VEC_SETTINGS_STRUCT_ITEMS
#undef VEC_SETTINGS_KEEP_ZERO_END
#undef VEC_SETTINGS_DEFAULT_SIZE

#define STR(string)             (Str){.s = (char *)string, .last = sizeof(string)/sizeof(*string)-1}
#define STR_L(string)           (Str){.s = (char *)string, .last = strlen(string ? string : "")}
#define STR_LL(string, length)  (Str){.s = (char *)string, .last = length}

#define STR_F(s)                (int)str_length(s), str_iter_begin(s)

#define STR_I0(str, i0)         (const Str){.s = (str).s, .first = ((str).first + i0 < (str).last) ? (str).first + i0 : (str).last, .last = (str).last}
#define STR_IE(str, iE)         (const Str){.s = (str).s, .first = (str).first, .last = (str).first + iE}

#define ERR_STR_CAT_BACK    "failed appending string to other string"
#define ERR_STR_FMT         "failed string formatting"
#define ERR_STR_COPY        "failed copying string"

#define ERR_str_copy(v1, v2) "failed copying string"

/* other functions */

void str_pop_back_char(Str *str);
void str_pop_back_word(Str *str);
void str_triml(Str *str);
void str_trimr(Str *str);
void str_trim(Str *str);

void str_remove_trailing_ch(Str *str, char ch, char ch_escape);

void str_cstr(const Str *str, char *cstr, size_t len);
void str_clear_to_last(Str *str);

#define ERR_str_fmt_va(str, format, argp) "failed formatting string"
ErrDecl str_fmt_va(Str *str, const char *format, va_list argp);
#define ERR_str_fmt(str, format, ...) "failed formatting string"
ErrDecl str_fmt(Str *str, const char *format, ...);
#define ERR_str_fmt_ext(ext, str) "failed formatting extension"
ErrDecl str_fmt_ext(Str *ext, const Str *str); // extract extension
#define ERR_str_fmt_noext(ext, str) "failed removing extension"
ErrDecl str_fmt_noext(Str *ext, const Str *str); // remove extension
#define ERR_str_fmt_dir(dir, str, up) "failed formatting directory"
ErrDecl str_fmt_dir(Str *dir, const Str *str, size_t up); // extract directory
#define ERR_str_fmt_nodir(nodir, str) "failed formatting without directory"
ErrDecl str_fmt_nodir(Str *nodir, const Str *str); // remove directory
#define ERR_str_fmt_basename(basename, str) "failed formatting basename"
ErrDecl str_fmt_basename(Str *basename, const Str *str); // remove extention+directory

#define ERR_str_fmt_line(line, str, i0, iE) "failed getting line (index %zu / length %zu)", i0, str_length(str)
ErrDecl str_fmt_line(Str *line, const Str *str, size_t i0, size_t *iE);
void str_get_line(const Str *str, size_t *i0, size_t *iE);

#define ERR_str_fmt_fgbg(out, text, ...) "failed applying foreground/background to string '%.*s'", STR_F(text)
ErrDecl str_fmt_fgbg(Str *out, const Str *text, const V3u8 fg, const V3u8 bg, bool bold, bool italic, bool underline);

#define ERR_STR_GET_STR     "failed getting string from user"
ErrDecl str_get_str(Str *str);

#define ERR_str_expand_path(path, base, home, ...) "failed expanding path"
//ErrDecl str_expand_path(Str *path, const Str *base, const Str *current, const Str *home);
ErrDecl str_expand_path(Str *path, const Str *base, const Str *home);

int str_cmp(const Str *a, const Str *b);
int str_cmp_sortable(const Str *a, const Str *b);
int str_cmp_ci(const Str *a, const Str *b);
int str_cmp_esci(const Str *a, const Str *b);
int str_cmp_ci_any(const Str *a, const Str **b, size_t len);
size_t str_count_overlap(const Str *a, const Str *b, bool ignorecase);
size_t str_find_substring(const Str *str, const Str *sub);
size_t str_find_any(const Str *str, const Str *any);
size_t str_find_nany(const Str *str, const Str *any);
// TODO size_t str_find_rnany(const Str *str, const Str *any);
size_t str_nch(const Str *str, char ch, size_t n);
size_t str_ch(const Str *str, char ch, size_t n);
size_t str_ch_from(const Str *str, char ch, size_t n, size_t from);
size_t str_ch_pair(const Str *str, char c1);
size_t str_find_ws(const Str *str);
size_t str_find_nws(const Str *str);
size_t str_find_rnws(const Str *str);
size_t str_rch(const Str *str, char ch, size_t n);
size_t str_rnch(const Str *str, char ch, size_t n);
size_t str_count_ch(const Str *str, char ch);
size_t str_irch(const Str *str, size_t iE, char ch, size_t n);
size_t str_hash(const Str *a);
size_t str_hash_ci(const Str *a);
size_t str_hash_esci(const Str *a);

Str str_splice(Str *to_splice, Str *prev_splice, char sep);

#define ERR_str_remove_escapes(out, in) "failed removing escape sequences"
ErrDecl str_remove_escapes(Str *out, Str *in);

#define STR_H
#endif

