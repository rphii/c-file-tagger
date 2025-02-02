#ifndef STR_H

/* {{{ header-only fluff */

/* {{{ basic includes */

#define STR_DEFAULT_SIZE 32

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <rphii/err.h>

/* }}} basic includes */

/* {{{ configuration, inclusion and de-configuration of vector */

typedef uint8_t V3u8[3];
typedef struct VStr VStr;

#include <rphii/vec.h>
//VEC_INCLUDE(VStr, vstr, struct Str, BY_REF, BASE);
//VEC_INCLUDE(VStr, vstr, struct Str, BY_REF, ERR);
VEC_INCLUDE(VStr, vstr, struct Str, BY_REF, BASE);
VEC_INCLUDE(VStr, vstr, struct Str, BY_REF, ERR);

//VEC_INCLUDE(VrStr, vrstr, struct Str, BY_REF, BASE);
//VEC_INCLUDE(VrStr, vrstr, struct Str, BY_REF, ERR);
VEC_INCLUDE(VrStr, vrstr, struct RStr, BY_REF, BASE);
VEC_INCLUDE(VrStr, vrstr, struct RStr, BY_REF, ERR);

#define VEC_SETTINGS_DEFAULT_SIZE STR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s
#include <rphii/vec.h>
VEC_INCLUDE(Str, str, char, BY_VAL, BASE);
VEC_INCLUDE(Str, str, char, BY_VAL, ERR);
#undef VEC_SETTINGS_DEFAULT_SIZE
#undef VEC_SETTINGS_KEEP_ZERO_END
#undef VEC_SETTINGS_STRUCT_ITEMS

/* }}} configuration, inclusion and de-configuration of vector */

/* {{{ utility macros */

#define STR(string)             (Str){.s = (char *)string, .last = sizeof(string)/sizeof(*string)-1}
#define STR_L(string)           (Str){.s = (char *)string, .last = strlen(string ? string : "")}
#define STR_LL(string, length)  (Str){.s = (char *)string, .last = length}

#define RSTR(string)             (RStr){.s = (char *)string, .last = sizeof(string)/sizeof(*string)-1}
#define RSTR_L(string)           (RStr){.s = (char *)string, .last = strlen(string ? string : "")}
#define RSTR_LL(string, length)  (RStr){.s = (char *)string, .last = length}

#define STR_F(s)                (int)str_length(s), str_iter_begin(s)
#define STR_I0(str, i0)         (const Str){.s = (str).s, .first = ((str).first + i0 < (str).last) ? (str).first + i0 : (str).last, .last = (str).last}
#define STR_IE(str, iE)         (const Str){.s = (str).s, .first = (str).first, .last = (str).first + iE}

#define RSTR_F(s)               (int)rstr_length(s), rstr_iter_begin(s)
#define RSTR_I0(str, i0)        (const RStr){.s = (str).s, .first = ((str).first + i0 < (str).last) ? (str).first + i0 : (str).last, .last = (str).last}
#define RSTR_IE(str, iE)        (const RStr){.s = (str).s, .first = (str).first, .last = (str).first + iE}

#define RSTR_STR(str)           (const Str){.s = (str).s, .first = (str).first, .last = (str).last}
#define STR_RSTR(str)           (const RStr){.s = (str).s, .first = (str).first, .last = (str).last}

/* }}} utility macros */

/* {{{ header helper macros */

#define STR_INCLUDE_MOD(type, name, ...)  \
    type str_##name(Str *str, ##__VA_ARGS__); \
    type rstr_##name(RStr *str, ##__VA_ARGS__);

#define STR_INCLUDE_CONST(type, name, ...)  \
    type str_##name(const Str str, ##__VA_ARGS__); \
    type rstr_##name(const RStr str, ##__VA_ARGS__);

#define PSTR_INCLUDE_CONST(type, name, ...)  \
    type str_p##name(const Str *str, ##__VA_ARGS__); \
    type rstr_p##name(const RStr *str, ##__VA_ARGS__);

/* }}} header helper macros */

/* }}} header-only fluff */

/* {{{ both , failable */

#define ERR_str_fmt_va(...) "failed formatting string"
ErrDecl str_fmt_va(Str *str, const char *format, va_list argp);

#define ERR_str_fmt(...) "failed formatting string"
ErrDecl str_fmt(Str *str, const char *format, ...);

#define ERR_str_fmt_fgbg(out, text, ...) "failed applying foreground/background to string '%.*s'", STR_F(text)
ErrDecl str_fmt_fgbg(Str *out, const Str *text, const V3u8 fg, const V3u8 bg, bool bold, bool italic, bool underline);

#define ERR_str_get_str(...) "failed getting string from user"
ErrDecl str_get_str(Str *str);

#define ERR_str_as_int(...) "error while converting string to number"
ErrDecl str_as_int(const Str str, ssize_t *out);
#define ERR_rstr_as_int(...) "error while converting string to number"
ErrDecl rstr_as_int(const RStr str, ssize_t *out);

#define ERR_str_as_bool(...) "error while converting string to bool"
ErrDecl str_as_bool(const Str str, bool *out, bool expand_pool);
#define ERR_rstr_as_bool(...) "error while converting string to bool"
ErrDecl rstr_as_bool(const RStr str, bool *out, bool expand_pool);



/*OK*/size_t str_writefunc(void *ptr, size_t size, size_t nmemb, Str *str);

/* }}} both , failable */

/* {{{ both , no-fail */

STR_INCLUDE_MOD(void, pop_back_char, RStr *);
STR_INCLUDE_MOD(void, pop_back_word, RStr *);
STR_INCLUDE_MOD(void, rremove_ch, char ch, char ch_escape);

STR_INCLUDE_CONST(void, cstr, char *cstr, size_t len);

STR_INCLUDE_CONST(RStr, triml);
STR_INCLUDE_CONST(RStr, trimr);
STR_INCLUDE_CONST(RStr, trim);

STR_INCLUDE_CONST(RStr, get_ext);         /* /home/user/file.test -> .test */
STR_INCLUDE_CONST(RStr, get_noext);       /* /home/user/file.test -> /home/user/file */
STR_INCLUDE_CONST(RStr, get_dir);         /* /home/user/file.test -> /home/user */
STR_INCLUDE_CONST(RStr, get_nodir);       /* /home/user/file.test -> file.test */
STR_INCLUDE_CONST(RStr, get_basename);    /* /home/user/file.test -> file */

STR_INCLUDE_CONST(size_t, find_nch, char ch, size_t n);
STR_INCLUDE_CONST(size_t, find_ch, char ch, size_t n);
STR_INCLUDE_CONST(size_t, find_ws);
STR_INCLUDE_CONST(size_t, find_nws);
STR_INCLUDE_CONST(size_t, count_overlap, const RStr b, bool ignorecase);
STR_INCLUDE_CONST(size_t, find_substr, RStr sub);

STR_INCLUDE_CONST(size_t, rfind_nch, char ch, size_t n);
STR_INCLUDE_CONST(size_t, rfind_ch, char ch, size_t n);
STR_INCLUDE_CONST(size_t, rfind_ws);
STR_INCLUDE_CONST(size_t, rfind_nws);

STR_INCLUDE_CONST(size_t, pair_ch, char c1);
STR_INCLUDE_CONST(size_t, count_ch, char ch);

STR_INCLUDE_CONST(size_t, hash);
STR_INCLUDE_CONST(size_t, hash_ci);
PSTR_INCLUDE_CONST(size_t, hash);
PSTR_INCLUDE_CONST(size_t, hash_ci);

STR_INCLUDE_CONST(RStr, splice, RStr *prev_splice, char sep);

int str_cmp(const Str a, const Str b);
int rstr_cmp(const RStr a, const RStr b);
int str_rstr_cmp(const Str a, const RStr b);

int str_pcmp(const Str *a, const Str *b);
int rstr_pcmp(const RStr *a, const RStr *b);
int str_rstr_pcmp(const Str *a, const RStr *b);

int str_cmp_sortable(const Str *a, const Str *b);
int rstr_cmp_sortable(const RStr *a, const RStr *b);
int str_rstr_cmp_sortable(const Str *a, const RStr *b);

int str_cmp_ci(const Str *a, const Str *b);
int rstr_cmp_ci(const RStr *a, const RStr *b);
int str_rstr_cmp_ci(const Str *a, const RStr *b);

/* }}} both , no-fail */

/* {{{ TODO implement for bot rstr and str */
size_t str_hash_esci(const Str *a);
int str_cmp_esci(const Str *a, const Str *b);
int str_cmp_ci_any(const Str *a, const Str **b, size_t len);
size_t str_find_any(const Str *str, const Str *any);
size_t str_find_nany(const Str *str, const Str *any);
// TODO size_t str_find_rnany(const Str *str, const Str *any);
#define ERR_str_remove_escapes(out, in) "failed removing escape sequences"
ErrDecl str_remove_escapes(Str *out, Str *in);
/* }}} TODO implement for bot rstr and str */


#define STR_H
#endif

