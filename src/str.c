#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "colorprint.h"
#include "err.h"

//#include "file.h"

/* {{{ source-only fluff */

/* inclusion and configuration of vector */
#include "str.h"

#define VEC_SETTINGS_DEFAULT_SIZE STR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s
VEC_IMPLEMENT(Str, str, char, BY_VAL, BASE, 0);
VEC_IMPLEMENT(Str, str, char, BY_VAL, ERR);
#undef VEC_SETTINGS_DEFAULT_SIZE
#undef VEC_SETTINGS_KEEP_ZERO_END
#undef VEC_SETTINGS_STRUCT_ITEMS

#include "vec.h"
#include "vector.h"

VEC_IMPLEMENT(VStr, vstr, Str, BY_REF, BASE, 0);
VEC_IMPLEMENT(VStr, vstr, Str, BY_REF, ERR);

VEC_IMPLEMENT(VrStr, vrstr, RStr, BY_REF, BASE, 0);
VEC_IMPLEMENT(VrStr, vrstr, RStr, BY_REF, ERR);
VEC_IMPLEMENT(VrStr, vrstr, RStr, BY_REF, SORT, rstr_cmp_sortable);

/* }}} source-only fluff */

/* {{{ Str-only , failable */

ErrImpl str_fmt_va(Str *str, const char *format, va_list argp) /*{{{*/
{
    ASSERT_ARG(str);
    ASSERT_ARG(format);
    va_list argp2;
    va_copy(argp2, argp);
    size_t len_app = (size_t)vsnprintf(0, 0, format, argp2);
    va_end(argp2);

    if((int)len_app < 0) {
        THROW("len_app is < 0!");
    }
    // calculate required memory
    size_t len_new = str->last + len_app;
    if(str_reserve(str, len_new)) {
        THROW("failed reserving %zu bytes!", len_new);
    }
    // actual append
    int len_chng = vsnprintf(&(str->s)[str->last], len_app + 1, format, argp);
    // check for success
    if(len_chng >= 0 && (size_t)len_chng <= len_app) {
        str->last += (size_t)len_chng; // successful, change length
    } else {
        THROW("len_chng is < 0!");
    }
    return 0;
error:
    ERR_PRINTF("failed formatting string: [%.*s] with format [%s]\n", STR_F(*str), format);
    return -1;
} /*}}}*/

ErrImpl str_fmt(Str *str, const char *format, ...) /*{{{*/
{
    ASSERT_ARG(str);
    ASSERT_ARG(format);
    if(!str) return -1;
    if(!format) return -1;
    // calculate length of append string
    va_list argp;
    va_start(argp, format);
    int result = str_fmt_va(str, format, argp);
    va_end(argp);
    return result;
} /*}}}*/

ErrImpl str_fmt_fgbg(Str *out, const Str *text, const V3u8 fg, const V3u8 bg, bool bold, bool italic, bool underline) { /*{{{*/
    ASSERT_ARG(out);
    ASSERT_ARG(text);
    bool do_fmt = ((fg || bg || bold || italic || underline));
    if(!do_fmt) {
        TRYC(str_fmt(out, "%.*s", STR_F(*text)));
        return 0;
    }
    char fmt[64] = {0}; /* theoretically 52 would be enough? */
    int len = sizeof(fmt)/sizeof(*fmt);
    int offs = 0;
    offs += snprintf(fmt, len, "%s", FS_BEG);
    if(fg) offs += snprintf(fmt + offs, len - offs, "%s", FS_FG3);
    if(bg) offs += snprintf(fmt + offs, len - offs, "%s", FS_BG3);
    if(bold) offs += snprintf(fmt + offs, len - offs, "%s", BOLD);
    if(italic) offs += snprintf(fmt + offs, len - offs, "%s", IT);
    if(underline) offs += snprintf(fmt + offs, len - offs, "%s", UL);
    snprintf(fmt + offs, len - offs, "%s", FS_END);
    if(fg && bg) { TRYC(str_fmt(out, fmt, fg[0], fg[1], fg[2], bg[0], bg[1], bg[2], STR_F(*text))); }
    else if(fg) {  TRYC(str_fmt(out, fmt, fg[0], fg[1], fg[2], STR_F(*text))); }
    else if(bg) {  TRYC(str_fmt(out, fmt, bg[0], bg[1], bg[2], STR_F(*text))); }
    else {         TRYC(str_fmt(out, fmt, STR_F(*text))); }
    return 0;
error:
    return -1;
} /*}}}*/

ErrImpl str_get_str(Str *str) /*{{{*/
{
    ASSERT_ARG(str);
    int err = 0;
    int c = 0;
    while((c = getchar()) != '\n' && c != EOF) {
        TRYC(str_fmt(str, "%c", (char)c));  /* append string */
    }
    if(!str->last && (!c || c == EOF || c == '\n')) {
        //THROW("an error"); /* TODO describe this error */
    }
clean:
    fflush(stdin);
    return err;
error: ERR_CLEAN;
} /*}}}*/

size_t str_writefunc(void *ptr, size_t size, size_t nmemb, Str *str) /*{{{*/
{
    TRYC(str_fmt(str, "%.*s", size*nmemb, ptr));
    return size*nmemb;
error:
    return -1;
} /*}}}*/

/* }}} Str-only , failable */

/* {{{ both , no-fail */


#define IMPL_STR_POP_BACK_CHAR(A, N) /*{{{*/ \
    void A##_pop_back_char(N *str, RStr *pop) { \
        ASSERT_ARG(str); \
        ASSERT_ARG(pop); \
        bool next; \
        do { \
            next = false; \
            char c = 0; \
            if(str->last > str->first) { \
                c =str->s[--str->last]; \
                next = (bool)((c & 0xC0) == 0x80); \
            } \
        } while(next); \
    }
IMPL_STR_POP_BACK_CHAR(str, Str);
IMPL_STR_POP_BACK_CHAR(rstr, RStr);
/*}}}*/

#define IMPL_STR_POP_BACK_WORD(A, N) /*{{{*/ \
    void A##_pop_back_word(N *str, RStr *pop) { \
        ASSERT_ARG(str); \
        ASSERT_ARG(pop); \
        size_t len = A##_length(*str); \
        if(len) { \
            int ws = isspace(A##_get_at(str, --len)); \
            while(len) { \
                char c = A##_get_at(str, --len); \
                int wsI = isspace(c); \
                if(ws && !wsI) { ++len; break; } \
                if(!ws && wsI) { ++len; break; } \
            } \
        } \
        str->last = str->first + len; \
    }
IMPL_STR_POP_BACK_WORD(str, Str);
IMPL_STR_POP_BACK_WORD(rstr, RStr);
/*}}}*/

#define IMPL_STR_RREMOVE_CH(A, N) /*{{{*/ \
    void A##_rremove_ch(N *str, char ch, char ch_escape) { \
        ASSERT_ARG(str); \
        while(A##_length(*str) && A##_get_back(str) == ch) { \
            if(A##_length(*str) >= 2) { \
                if(A##_get_at(str, A##_length(*str) - 2) == ch_escape) { \
                    break; \
                } \
            } \
            --str->last; \
        } \
    }
IMPL_STR_RREMOVE_CH(str, Str);
IMPL_STR_RREMOVE_CH(rstr, RStr);
/*}}}*/

#define STR_IMPL_CSTR(A, N, FMT) /*{{{*/ \
    void A##_cstr(const N str, char *cstr, size_t len) {  \
        cstr[0] = 0; \
        snprintf(cstr, len, "%.*s", FMT(str)); \
    }
STR_IMPL_CSTR(str, Str, STR_F);
STR_IMPL_CSTR(rstr, RStr, RSTR_F);
/*}}}*/

#define IMPL_STR_TRIML(A, N) /*{{{*/ \
    RStr A##_triml(const N str) { \
        RStr result = A##_rstr(str); \
        while(result.first < result.last) { \
            char c = result.s[result.first]; \
            if(!isspace(c)) break; \
            ++result.first; \
        } \
        return result; \
    }
IMPL_STR_TRIML(str, Str);
IMPL_STR_TRIML(rstr, RStr);
/*}}}*/

#define IMPL_STR_TRIMR(A, N) /*{{{*/ \
    RStr A##_trimr(const N str) { \
        RStr result = A##_rstr(str); \
        while(result.last > result.first) { \
            char c = result.s[result.last - 1]; \
            if(!isspace(c)) break; \
            --result.last; \
        } \
        return result; \
    }
IMPL_STR_TRIMR(str, Str);
IMPL_STR_TRIMR(rstr, RStr);
/*}}}*/

#define IMPL_STR_TRIM(A, N) /*{{{*/ \
    RStr A##_trim(N str) { \
        RStr result = A##_triml(str); \
        result = rstr_trimr(result); \
        return result; \
    }
IMPL_STR_TRIM(str, Str);
IMPL_STR_TRIM(rstr, RStr);
/*}}}*/

#define IMPL_STR_GET_EXT(A, N) /*{{{*/ \
    RStr A##_get_ext(const N str) { \
        RStr result = A##_rstr(str); \
        size_t len = rstr_length(result); \
        if(len) { \
            size_t i = rstr_rfind_ch(result, '.', 0); \
            if(i < len) { \
                /* in case we have something like: file.dir/filename -> / is after . */ \
                size_t j = rstr_rfind_ch(result, PLATFORM_CH_SUBDIR, 0); \
                if((j < len && j < i) || (j == len)) { \
                    result.first = str.first + i; \
                    result.last = result.first + (len - i); \
                } \
            } \
        } \
        return result; \
    }
IMPL_STR_GET_EXT(str, Str);
IMPL_STR_GET_EXT(rstr, RStr);
/*}}}*/

#define IMPL_STR_GET_NOEXT(A, N) /*{{{*/ \
    RStr A##_get_noext(const N str) { \
        RStr result = A##_rstr(str); \
        size_t len = rstr_length(result); \
        if(len) { \
            size_t i = rstr_rfind_ch(result, '.', 0); \
            if(i < len) { \
                /* in case we have something like: file.dir/filename -> / is after . */ \
                size_t j = rstr_rfind_ch(result, PLATFORM_CH_SUBDIR, 0); \
                if((j < len && j < i) || (j == len)) { \
                    result.last = i; \
                } \
            } \
        } \
        return result; \
    }
IMPL_STR_GET_NOEXT(str, Str);
IMPL_STR_GET_NOEXT(rstr, RStr);
/*}}}*/

#define IMPL_STR_GET_DIR(A, N) /*{{{*/ \
    RStr A##_get_dir(const N str) { \
        RStr result = A##_rstr(str); \
        size_t len = rstr_length(result); \
        if(len) { \
            size_t i0 = rstr_rfind_ch(result, '/', 0); \
            if(i0 < len && PLATFORM_CH_SUBDIR != '/') { \
                i0 = rstr_rfind_ch(result, PLATFORM_CH_SUBDIR, 0); \
            } \
            /*if(i0 < len) ++i0;*/ \
            else if(i0 >= len) i0 = 0; \
            result.last = str.first + i0; \
        } \
        return result; \
    }
IMPL_STR_GET_DIR(str, Str);
IMPL_STR_GET_DIR(rstr, RStr);
/*}}}*/

#define IMPL_STR_GET_NODIR(A, N) /*{{{*/ \
    RStr A##_get_nodir(const N str) { \
        RStr result = A##_rstr(str); \
        size_t len = rstr_length(result); \
        if(len) { \
            size_t i0 = rstr_rfind_ch(result, '/', 0); \
            if(i0 < len && PLATFORM_CH_SUBDIR != '/') { \
                i0 = rstr_rfind_ch(result, PLATFORM_CH_SUBDIR, 0); \
            } \
            if(i0 < len) ++i0; \
            else if(i0 >= len) i0 = 0; \
            result.first = str.first + i0; \
        } \
        return result; \
    }
IMPL_STR_GET_NODIR(str, Str);
IMPL_STR_GET_NODIR(rstr, RStr);
/*}}}*/

#define IMPL_STR_GET_BASENAME(A, N) /*{{{*/ \
    RStr A##_get_basename(const N str) { \
        RStr result = A##_rstr(str); \
        size_t len = rstr_length(result); \
        if(len) { \
            size_t iE = rstr_rfind_ch(result, '.', 0); \
            size_t i0 = rstr_rfind_ch(result, '/', 0); \
            if(i0 < len && PLATFORM_CH_SUBDIR != '/') { \
                i0 = rstr_rfind_ch(result, PLATFORM_CH_SUBDIR, 0); \
            } \
            if(i0 < len) ++i0; \
            else if(i0 >= len) i0 = 0; \
            result.first = str.first + i0; \
            result.last = result.first + (iE - i0); \
            /*TRYC(str_fmt(basename, "%.*s", (int)(iE - i0), str_iter_at(str, i0)));*/ \
        } \
        return result; \
    }
IMPL_STR_GET_BASENAME(str, Str);
IMPL_STR_GET_BASENAME(rstr, RStr);
/*}}}*/

#define IMPL_STR_FIND_NCH(A, N) /*{{{*/ \
    size_t A##_find_nch(const N str, char ch, size_t n) {  \
        size_t ni = 0; \
        for(size_t i = 0; i < A##_length(str); ++i) { \
            char c = A##_get_at(&str, i); \
            if(c != ch) { \
                if(ni == n) return i; \
                ++ni; \
            } \
        } \
        return A##_length(str); \
    }
IMPL_STR_FIND_NCH(str, Str);
IMPL_STR_FIND_NCH(rstr, RStr);
/*}}}*/

#define IMPL_STR_FIND_CH(A, N) /*{{{*/ \
size_t A##_find_ch(const N str, char ch, size_t n) { \
        size_t ni = 0; \
        for(size_t i = 0; i < A##_length(str); ++i) { \
            char c = A##_get_at(&str, i); \
            if(c == ch) { \
                if(ni == n) return i; \
                ++ni; \
            } \
        } \
        return A##_length(str); \
    } 
IMPL_STR_FIND_CH(str, Str);
IMPL_STR_FIND_CH(rstr, RStr);
/*}}}*/

#define IMPL_STR_FIND_WS(A, N) /*{{{*/ \
    size_t A##_find_ws(const N str) { \
        for(size_t i = 0; i < A##_length(str); ++i) { \
            char c = A##_get_at(&str, i); \
            if(isspace(c)) return i; \
        } \
        return A##_length(str); \
    } 
IMPL_STR_FIND_WS(str, Str);
IMPL_STR_FIND_WS(rstr, RStr);
/*}}}*/

#define IMPL_STR_FIND_NWS(A, N) /*{{{*/ \
    size_t A##_find_nws(const N str) { \
        for(size_t i = 0; i < A##_length(str); ++i) { \
            char c = A##_get_at(&str, i); \
            if(!isspace(c)) return i; \
        } \
        return A##_length(str); \
    }
IMPL_STR_FIND_NWS(str, Str);
IMPL_STR_FIND_NWS(rstr, RStr);
/*}}}*/


#define IMPL_STR_RFIND_NCH(A, N) /*{{{*/ \
    size_t A##_rfind_nch(const N str, char ch, size_t n) {  \
        size_t ni = 0; \
        for(size_t i = A##_length(str); i > 0; --i) { \
            char c = A##_get_at(&str, i - 1); \
            if(c != ch) { \
                if(ni == n) return i - 1; \
                ++ni; \
            } \
        } \
        return 0; /*TODO:PANIC?!!??!?! A##_length(str);*/ \
    }
IMPL_STR_RFIND_NCH(str, Str);
IMPL_STR_RFIND_NCH(rstr, RStr);
/*}}}*/

#define IMPL_STR_RFIND_CH(A, N) /*{{{*/ \
    size_t A##_rfind_ch(const N str, char ch, size_t n) { \
        size_t ni = 0; \
        for(size_t i = A##_length(str); i > 0; --i) { \
            char c = A##_get_at(&str, i - 1); \
            if(c == ch) { \
                if(ni == n) return i - 1; \
                ++ni; \
            } \
        } \
        return A##_length(str); \
    } 
IMPL_STR_RFIND_CH(str, Str);
IMPL_STR_RFIND_CH(rstr, RStr);
/*}}}*/

#define IMPL_STR_RFIND_WS(A, N) /*{{{*/ \
    size_t A##_rfind_ws(const N str) { \
        for(size_t i = A##_length(str); i > 0; --i) { \
            char c = A##_get_at(&str, i - 1); \
            if(isspace(c)) return i - 1; \
        } \
        return A##_length(str); \
    }
IMPL_STR_RFIND_WS(str, Str);
IMPL_STR_RFIND_WS(rstr, RStr);
/*}}}*/

#define IMPL_STR_RFIND_NWS(A, N) /*{{{*/ \
    size_t A##_rfind_nws(const N str) { \
        for(size_t i = A##_length(str); i > 0; --i) { \
            char c = A##_get_at(&str, i - 1); \
            if(!isspace(c)) return i - 1; \
        } \
        return A##_length(str); \
    }
IMPL_STR_RFIND_NWS(str, Str);
IMPL_STR_RFIND_NWS(rstr, RStr);
/*}}}*/

#define IMPL_STR_COUNT_OVERLAP(A, N) /*{{{*/ \
    inline size_t A##_count_overlap(const N a, const RStr b, bool ignorecase) { \
        size_t overlap = 0; \
        size_t len = A##_length(a) > rstr_length(b) ? rstr_length(b) : A##_length(a); \
        if(!ignorecase) { \
            for(size_t i = 0; i < len; ++i) { \
                char ca = A##_get_at(&a, i); \
                char cb = rstr_get_at(&b, i); \
                if(ca == cb) ++overlap; \
                else break; \
            } \
        } else { \
            for(size_t i = 0; i < len; ++i) { \
                int ca = tolower(A##_get_at(&a, i)); \
                int cb = tolower(rstr_get_at(&b, i)); \
                if(ca == cb) ++overlap; \
                else break; \
            } \
        } \
        return overlap; \
    }
IMPL_STR_COUNT_OVERLAP(str, Str);
IMPL_STR_COUNT_OVERLAP(rstr, RStr);
/*}}}*/

#define IMPL_STR_FIND_SUBSTR(A, N) /*{{{*/ \
    inline size_t A##_find_substr(const N str, const RStr sub) { \
        /* basic checks */ \
        if(!rstr_length(sub)) return 0; \
        if(rstr_length(sub) > A##_length(str)) { \
            return A##_length(str); \
        } \
        /* store original indices */ \
        N ref = str; \
        /* check for substring */ \
        size_t i = 0; \
        while(rstr_length(sub) <= A##_length(ref)) { \
            size_t overlap = A##_count_overlap(ref, sub, true); \
            if(overlap == rstr_length(sub)) { \
                return i; \
            } else { \
                i += overlap + 1; \
                ref.first += overlap + 1; \
            } \
        } \
        /* restore original */ \
        return A##_length(str); \
    }
IMPL_STR_FIND_SUBSTR(str, Str);
IMPL_STR_FIND_SUBSTR(rstr, RStr);
/*}}}*/


#define IMPL_STR_PAIR_CH(A, N) /*{{{*/ \
    size_t A##_pair_ch(const N str, char c1) { \
        if(!A##_length(str)) return A##_length(str); \
        size_t level = 1; \
        char c0 = A##_get_at(&str, 0); \
        for(size_t i = 1; i < A##_length(str); ++i) { \
            char c = A##_get_at(&str, i); \
            if(c == c0) level++; \
            else if(c == c1) level--; \
            if(level <= 0) return i; \
        } \
        return A##_length(str); \
    }
IMPL_STR_PAIR_CH(str, Str);
IMPL_STR_PAIR_CH(rstr, RStr);
/*}}}*/

#define IMPL_STR_COUNT_CH(A, N) /*{{{*/ \
size_t A##_count_ch(const N str, char ch) {  \
        size_t result = 0; \
        if(str.first < str.last) { /* TODO: add this to basically every string utility function :) */ \
            for(size_t i = 0; i < A##_length(str); ++i) { \
                char c = A##_get_at(&str, i); \
                if(c == ch) ++result; \
            } \
        } \
        return result; \
    }
IMPL_STR_COUNT_CH(str, Str);
IMPL_STR_COUNT_CH(rstr, RStr);
/*}}}*/


#define IMPL_STR_HASH(A, N, P, S) /*{{{*/ \
    size_t A##_##S##hash(const N P a) { \
        size_t hash = 5381; \
        size_t i = 0; \
        while(i < A##_length(P a)) { \
            unsigned char c = (unsigned char)A##_get_at(P &a, i++); \
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */ \
        } \
        return hash; \
    }
IMPL_STR_HASH(str, Str, *, p);
IMPL_STR_HASH(rstr, RStr, *, p);
IMPL_STR_HASH(str, Str,  ,  );
IMPL_STR_HASH(rstr, RStr,  ,  );
/*}}}*/ 

#define IMPL_STR_HASH_CI(A, N, P, S) /*{{{*/ \
    size_t A##_##S##hash_ci(const N P a) { \
        size_t hash = 5381; \
        size_t i = 0; \
        while(i < A##_length(P a)) { \
            unsigned char c = (unsigned char)tolower(A##_get_at(P &a, i++)); \
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */ \
        } \
        return hash; \
    }
IMPL_STR_HASH_CI(str, Str, *, p);
IMPL_STR_HASH_CI(rstr, RStr, *, p);
IMPL_STR_HASH_CI(str, Str, , );
IMPL_STR_HASH_CI(rstr, RStr, , );
/*}}}*/

#define IMPL_STR_SPLICE(A, N) /*{{{*/ \
    RStr A##_splice(const N to_splice, RStr *prev_splice, char sep) { \
        RStr result = A##_rstr(to_splice); \
        RStr *prev = prev_splice; \
        if(prev && prev->s) { \
            size_t from = rstr_iter_begin(*prev) - A##_iter_begin(to_splice); \
            RStr search = RSTR_I0(A##_rstr(to_splice), from); \
            result.first += rstr_find_ch(search, sep, 0) + from; /*TODO is this really += ??*/ \
            if((size_t)(rstr_iter_begin(result) - A##_iter_begin(to_splice)) < A##_length(to_splice)) { \
                ++result.first; \
            } \
        } \
        result.last = result.first + rstr_find_ch(result, sep, 0); \
        return result; \
    }
IMPL_STR_SPLICE(str, Str);
IMPL_STR_SPLICE(rstr, RStr);
/*}}}*/

#define IMPL_STR_AS_INT(A, N, F) /*{{{*/ \
    int A##_as_int(const N str, ssize_t *out) { \
        ASSERT_ARG(out); \
        char *endptr; \
        ssize_t result = strtoll(A##_iter_begin(str), &endptr, 0); \
        if(endptr != A##_iter_end(&str)) THROW("failed conversion to number: '%.*s'", F(str)); \
        *out = result; \
        return 0; \
    error: \
        return -1; \
    }
IMPL_STR_AS_INT(str, Str, STR_F);
IMPL_STR_AS_INT(rstr, RStr, RSTR_F);
/*}}}*/

#define IMPL_STR_AS_BOOL(A, N, F) /*{{{*/ \
    int A##_as_bool(const N str, bool *out, bool expand_pool) { \
        ASSERT_ARG(out); \
        RStr in = A##_rstr(str); \
        RStr val_true[3] = { \
            RSTR("true"), \
            RSTR("yes"), \
            RSTR("enable"), \
        }; \
        RStr val_false[3] = { \
            RSTR("false"), \
            RSTR("no"), \
            RSTR("disable"), \
        }; \
        bool result = false; \
        for(size_t i = 0; i < 3; ++i) { \
            if(i && !expand_pool) THROW("invalid bool: '%.*s'", F(str)); \
            if(!rstr_cmp(val_true[i], in)) { result = true; break; } \
            if(!rstr_cmp(val_false[i], in)) { result = false; break; } \
        } \
        *out = result; \
        return 0; \
    error: \
        return -1; \
    }
IMPL_STR_AS_BOOL(str, Str, STR_F);
IMPL_STR_AS_BOOL(rstr, RStr, RSTR_F);
/*}}}*/

#define IMPL_STR_CMP(A, A1, N1, A2, N2, P, S) /*{{{*/ \
    int A##_##S##cmp(const N1 P a, const N2 P b) { \
        size_t la = A1##_length(P a); \
        size_t lb = A2##_length(P b); \
        if(la != lb) return -1; \
        int result = memcmp(A1##_iter_begin(P a), A2##_iter_begin(P b), la); \
        return result; \
    } 
IMPL_STR_CMP(str, str, Str, str, Str, , );
IMPL_STR_CMP(str, str, Str, str, Str, *, p);
IMPL_STR_CMP(rstr, rstr, RStr, rstr, RStr, , );
IMPL_STR_CMP(rstr, rstr, RStr, rstr, RStr, *, p);
IMPL_STR_CMP(str_rstr, str, Str, rstr, RStr, , );
IMPL_STR_CMP(str_rstr, str, Str, rstr, RStr, *, p);
/*}}}*/

#define IMPL_STR_CMP_SORTABLE(A, A1, N1, A2, N2) /*{{{*/ \
    int A##_cmp_sortable(const N1 *a, const N2 *b) { \
        ASSERT_ARG(a); \
        ASSERT_ARG(b); \
        size_t la = A1##_length(*a); \
        size_t lb = A2##_length(*b); \
        int result = -1; \
        /*printff("CMP %zu[%.*s] %zu[%.*s]", A##_length(a), STR_F(a), A##_length(b), STR_F(b));*/\
        if(la != lb) { \
            size_t less = la < lb ? la : lb; \
            result = memcmp(A1##_iter_begin(*a), A2##_iter_begin(*b), less); \
            if(!result) { \
                result = la - lb; \
            } \
        } else { \
            result = memcmp(A1##_iter_begin(*a), A2##_iter_begin(*b), la); \
        } \
        return result; \
    } 
IMPL_STR_CMP_SORTABLE(str, str, Str, str, Str);
IMPL_STR_CMP_SORTABLE(rstr, rstr, RStr, rstr, RStr);
IMPL_STR_CMP_SORTABLE(str_rstr, str, Str, rstr, RStr);
/*}}}*/

#define IMPL_STR_CMP_CI(A, A1, N1, A2, N2) /*{{{*/ \
    int A##_cmp_ci(const N1 *a, const N2 *b) { \
        ASSERT_ARG(a); \
        ASSERT_ARG(b); \
        if(A1##_length(*a) != A2##_length(*b)) return -1; \
        for (size_t i = 0; i < A1##_length(*a); ++i) { \
            int d = tolower(A1##_get_at(a, i)) - tolower(A2##_get_at(b, i)); \
            if (d != 0) return d; \
        } \
        return 0; \
    } 
IMPL_STR_CMP_CI(str, str, Str, str, Str);
IMPL_STR_CMP_CI(rstr, rstr, RStr, rstr, RStr);
IMPL_STR_CMP_CI(str_rstr, str, Str, rstr, RStr);
/*}}}*/

/* }}} both , no-fail */

/* {{{ TODO implement for bot rstr and str */

size_t str_hash_esci(const Str *a) { /*{{{*/
    ASSERT_ARG(a);
    size_t hash = 5381;
    size_t i = 0;
    int stage = 0;
    while(i < str_length(*a)) {
        unsigned char c = (unsigned char)tolower(str_get_at(a, i++));
        if(!stage) {
            if(c == '\033') ++stage;
            else hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }
        else if(stage == 1 && c == '[') ++stage;
        else if(stage == 2 && c == 'm') stage = 0;
    }
    return hash;
} /*}}}*/

int str_cmp_esci(const Str *a, const Str *b) { /*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
#if 1
    size_t ia = 0, ioff = 0; /* I am abusing this as signed even tho it's unsigned - trust me it'll 100% work! */
    int sa = 0, sb = 0;
    char ca = 0, cb = 0;
    //printf("COMPARE: %.*s[%zu] .. %.*s[%zu]\n", STR_F(a), str_length(a), STR_F(b), str_length(b));
    while(true) {
        /* breaking condition - are we at the end of the string? */
        if(ia >= str_length(*a) && ia+ioff >= str_length(*b)) break;
        /* get chars at current pos */
        if(ia < str_length(*a)) {
            ca = str_get_at(a, ia);
        }
        if(ia+ioff < str_length(*b)) {
            cb = str_get_at(b, ia+ioff);
        }
        //printf("ia %zu, ioff %zi, ca '%c', cb '%c'\n", ia, ioff, ca, cb);
        /* check state */
        if(!sa && !sb) {
            if(ca == '\033') ++sa;
            if(cb == '\033') ++sb;
            if(!sa && !sb) {
                /* is one string at the end? */
                if(ia >= str_length(*a) && ia+ioff >= str_length(*b)) return -1;
                /* both chars can be compared normally */
                int d = tolower(ca) - tolower(cb);
                if (d != 0) return d;
            }
            /* next indices */
            if((!sa && !sb) || (sa && sb)) {
                ia++;
            } else if(sa && !sb) {
                ia++;
                ioff--;
            } else if(!sa && sb) {
                ioff++;
            }
        } else {
            /* next indices */
            if((!sa && !sb) || (sa && sb)) {
                ia++;
            } else if(sa && !sb) {
                ia++;
                ioff--;
            } else if(!sa && sb) {
                ioff++;
            }
            /* we can't compare */
            if(sa == 1 && ca == '[') ++sa;
            else if(sa == 2 && ca == 'm') sa = 0;
            if(sb == 1 && cb == '[') ++sb;
            else if(sb == 2 && cb == 'm') sb = 0;
        }
    }
    /* strings were not equally long */
    if(ia < str_length(*a) || (ia+ioff < str_length(*b))) {
        return -1;
    }
#else
    size_t ia = 0, ib = 0;
    int sa = 0, sb = 0;
    char ca = 0, cb = 0;
    while(true) {
        if(!sa && !sb) {
            if(ca == '\033') ++sa;
            else ca = str_get_at(a, ia++);
            if(cb == '\033') ++sb;
            else cb = str_get_at(b, ib++);
            if(!sa && !sb) {
                int d = tolower(str_get_at(a, ia)) - tolower(str_get_at(b, ib));
                if (d != 0) return d;
            }
        }
        if(sa == 1 && ca == '[') ++sa;
        else if(sa == 2 && ca == 'm') sa = 0;
        if(sb == 1 && cb == '[') ++sb;
        else if(sb == 2 && cb == 'm') sb = 0;
    }
    if(ia != str_length(a) || ib != str_length(b)) return -1;
#endif
    return 0;
} /*}}}*/

int str_cmp_ci_any(const Str *a, const Str **b, size_t len) { /*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    for (size_t i = 0; i < len; ++i) {
        const Str *bb = b[i];
        int result = str_cmp_ci(a, bb);
        if(!result) return 0;
    }
    return -1;
} /*}}}*/

size_t str_find_any(const Str *str, const Str *any) { /*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(any);
    size_t result = str_length(*str);
    for(size_t i = 0; i < str_length(*any); ++i) {
        size_t temp = str_find_ch(*str, str_get_at(any, i), 0);
        if(temp < result) result = temp;
    }
    return result;
} /*}}}*/

size_t str_find_nany(const Str *str, const Str *any) { /*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(any);
    for(size_t i = 0; i < str_length(*str); ++i) {
        size_t temp = str_find_ch(*any, str_get_at(str, i), 0);
        if(temp >= str_length(*any)) return i;
    }
    return str_length(*str);
} /*}}}*/

ErrDecl str_remove_escapes(Str *restrict out, Str *restrict in) { /*{{{*/
    ASSERT(out, ERR_NULL_ARG);
    ASSERT(in, ERR_NULL_ARG);
    bool skip = 0;
    size_t iX = 0;
    char c_last = 0;
    for(size_t i = 0; i < str_length(*in); i++) {
        char c = str_get_at(in, i);
        if(!skip) {
            if(c == '\033') {
                TRYC(str_fmt(out, "%.*s", (int)(i - iX), str_iter_at(in, iX)));
                skip = true;
#if 0
            } else if(c == '\'') {
                TRY(str_fmt(out, "%.*s\'\\\'\'", (int)(i - iX), str_iter_at(in, iX)), ERR_STR_FMT);
                iX = i + 1;
#endif
            } else if(!isspace(c_last) && isspace(c)) {
                TRYC(str_fmt(out, "%.*s ", (int)(i - iX), str_iter_at(in, iX)));
                iX = i + 1;
            } else if(isspace(c_last) && isspace(c)) {
                iX = i + 1;
            }
            if(!skip) c_last = c;
        } else {
            if(c == 'm') {
                iX = i + 1;
                skip = false;
            }
        }
    }
    if(!skip) {
        TRYC(str_fmt(out, "%.*s", (int)(str_length(*in) - iX), str_iter_at(in, iX)));
    }
    return 0;
error:
    return -1;
} /*}}}*/

/* }}} TODO implement for bot rstr and str */

