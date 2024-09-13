#ifndef STR_H

#define STR_DEFAULT_SIZE 32

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "err.h"

typedef uint8_t V3u8[3];

#if 1

typedef struct Str {
    union {
        struct {
            char *s;
            size_t first;
            size_t last;
            size_t cap;
        };
        struct {
            char s[sizeof(size_t) * 3 + sizeof(char *) - 1];
            uint8_t f;
        } small;
    };
} Str;

#define STR_MASK_ACTIVE     0x80
#define STR_MASK_LEN        0x7F

#define STR_SMALL_LEN_SET(str, len)     do { STR_SMALL_LEN_CLEAR(str); (str)->small.f |= (len & STR_MASK_LEN); } while(0)
#define STR_SMALL_LEN_ADD(str, n)       (str)->small.f += (n)
#define STR_SMALL_LEN_CLEAR(str)        ((str)->small.f &= ~STR_MASK_LEN)
#define STR_SMALL_ENABLE(str)           ((str)->small.f |= STR_MASK_ACTIVE)
#define STR_SMALL_DISABLE(str)          ((str)->small.f &= ~STR_MASK_ACTIVE)
#define STR_SMALL_ACTIVE(str)           (bool)((str)->small.f & STR_MASK_ACTIVE)
#define STR_SMALL_LEN(str)              (uint8_t)((str)->small.f & STR_MASK_LEN)

void str_free(Str *str);
void str_clear(Str *str);
void str_zero(Str *str);
size_t str_length(const Str *str);

size_t str_alloced_bytes_in_use(Str *str);

#define ERR_str_reserve(...) "failed reserving memory"
ErrDecl str_copy(Str *dst, const Str *src);
ErrDecl str_reserve(Str *str, size_t cap);
ErrDecl str_push_back(Str *str, char c);
ErrDecl str_push_front(Str *str, char c);

void str_print_verbose(Str *str);

void str_set_at(Str *str, size_t i, char c);

#define ERR_str_extend_back(...) "asdf"
ErrDecl str_extend_back(Str *str, Str *add);

void str_pop_back(Str *str, char *c);
void str_pop_front(Str *str, char *c);

char str_get_front(const Str *str);
char str_get_back(const Str *str);
char str_get_at(const Str *str, size_t i);

char *str_iter_begin(const Str *str);
char *str_iter_end(const Str *str);
char *str_iter_at(const Str *str, size_t i);

#define STR_IS_SHORT(str)               (0)
//#define STR_IS_SHORT(str)               ((str)->w[STR_SHORT_CAP] & 0x80)
#define STR_SHORT_GET_LEN(str)          ((str)->w[STR_SHORT_CAP] & 0x7F)
#define STR_SHORT_CAP                   (sizeof(Str) - 1)

#define STR_SHORT_SET_LEN(str, len)     ((str)->w[STR_SHORT_CAP] = (len & 0x7F))

#define STR_SHORT_ENABLE(str)           ((str)->w[STR_SHORT_CAP] |= 0x80)
#define STR_SHORT_DISABLE(str)          ((str)->w[STR_SHORT_CAP] &= ~0x80)

#define STR_LONG_DEFAULT_SIZE           (2*sizeof(Str))

#else
/* configuration, inclusion and de-configuration of vector */

#define VEC_SETTINGS_DEFAULT_SIZE STR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s
#include "vec.h"

VEC_INCLUDE(Str, str, char, BY_VAL, BASE);
#endif

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

