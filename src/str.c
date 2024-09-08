#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "colorprint.h"
#include "err.h"
#include "vector.h"

/* inclusion and configuration of vector */
#include "str.h"
#include "file.h"
#include "platform.h"

#define VEC_SETTINGS_DEFAULT_SIZE STR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s

VEC_IMPLEMENT(Str, str, char, BY_VAL, BASE, 0);

/* other functions */

// basic, no fail, manipulation function {{{

void str_pop_back_char(Str *str) //{{{
{
    ASSERT_ARG(str);
    bool next;
    do {
        next = false;
        size_t len = str_length(str);
        char c = 0;
        if(len) {
            str_pop_back(str, &c);
            next = (bool)((c & 0xC0) == 0x80);
        }
    } while(next);
} //}}}

void str_pop_back_word(Str *str) //{{{
{
    ASSERT_ARG(str);
    size_t len = str_length(str);
    if(len) {
        int ws = isspace(str_get_at(str, --len));
        while(len) {
            char c = str_get_at(str, --len);
            int wsI = isspace(c);
            if(ws && !wsI) { ++len; break; }
            if(!ws && wsI) { ++len; break; }
        }
    }
    str->last = str->first + len;
} //}}}

void str_triml(Str *str) //{{{
{
    ASSERT_ARG(str);
    while(str->first < str->last) {
        char c = str->s[str->first];
        if(!isspace(c)) break;
        ++str->first;
    }
} //}}}

void str_trimr(Str *str) //{{{
{
    ASSERT_ARG(str);
    while(str->last > str->first) {
        char c = str->s[str->last - 1];
        if(!isspace(c)) break;
        --str->last;
    }
} //}}}

void str_trim(Str *str) //{{{
{
    ASSERT_ARG(str);
    str_triml(str);
    str_trimr(str);
} //}}}

// }}}

// pseudo directory {{{

void str_cstr(const Str *str, char *cstr, size_t len) {
    ASSERT_ARG(str);
    ASSERT_ARG(cstr);
    cstr[0] = 0;
    snprintf(cstr, len, "%.*s", STR_F(str));
}

void str_clear_to_last(Str *str) {
    ASSERT_ARG(str);
    str->first = str->last;
}

inline int str_fmt_va(Str *str, const char *format, va_list argp) //{{{
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
    ERR_PRINTF("failed formatting string: [%.*s] with format [%s]\n", STR_F(str), format);
    return -1;
} //}}}

int str_fmt(Str *str, const char *format, ...) //{{{
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
} //}}}

ErrDecl str_fmt_ext(Str *ext, const Str *str) //{{{
{
    ASSERT_ARG(str);
    ASSERT_ARG(ext);
    size_t len = str_length(str);
    if(len) {
        size_t i = str_rch(str, '.', 0);
        if(i < len) {
            /* in case we have something like: file.dir/filename -> / is after . */
            size_t j = str_rch(str, PLATFORM_CH_SUBDIR, 0);
            if((j < len && j < i) || (j == len)) {
                TRYC(str_fmt(ext, "%.*s", (int)(len - i), str_iter_at(str, i)));
            }
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl str_fmt_noext(Str *ext, const Str *str) //{{{
{
    ASSERT_ARG(str);
    ASSERT_ARG(ext);
    size_t len = str_length(str);
    if(len) {
        size_t iE = str_rch(str, '.', 0);
        TRYC(str_fmt(ext, "%.*s", (int)(iE), str_iter_begin(str)));
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl str_fmt_basename(Str *basename, const Str *str) //{{{
{
    ASSERT_ARG(str);
    ASSERT_ARG(basename);
    size_t len = str_length(str);
    if(len) {
        size_t iE = str_rch(str, '.', 0);
        size_t i0 = str_rch(str, '/', 0);
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str_rch(str, PLATFORM_CH_SUBDIR, 0);
        }
        if(i0 < len) ++i0;
        else if(i0 >= len) i0 = 0;
        TRYC(str_fmt(basename, "%.*s", (int)(iE - i0), str_iter_at(str, i0)));
    }
    return 0;
error:
    return -1;
} //}}}

// TODO: what if up is larger than the directory string? what should be returned then??
ErrDecl str_fmt_dir(Str *dir, const Str *str, size_t up) //{{{
{
    ASSERT_ARG(str);
    ASSERT_ARG(dir);
    size_t len = str_length(str);
    size_t len_dir = str_length(dir);
    if(len) {
        size_t i = str_rch(str, '/', up);
        if(i < len) {
            TRYC(str_fmt(dir, "%.*s", (int)(i+1), str_iter_begin(str)));
        }
        else if(PLATFORM_CH_SUBDIR != '/') {
            i = str_rch(str, PLATFORM_CH_SUBDIR, up);
            if(i < len) {
                TRYC(str_fmt(dir, "%.*s", (int)(i+1), str_iter_begin(str)));
            }
        }
    }
    if(len_dir == str_length(dir)) {
        TRYC(str_fmt(dir, "."));
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl str_fmt_nodir(Str *nodir, const Str *str) //{{{
{
    ASSERT_ARG(str);
    ASSERT_ARG(nodir);
    size_t len = str_length(str);
    if(len) {
        size_t i0 = str_rch(str, '/', 0);
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str_rch(str, PLATFORM_CH_SUBDIR, 0);
        }
        if(i0 < len) ++i0;
        else if(i0 >= len) i0 = 0;
        TRYC(str_fmt(nodir, "%.*s", (int)(len - i0), str_iter_at(str, i0)));
    }
    return 0;
error:
    return -1;
} //}}}

//}}}

int str_get_str(Str *str) //{{{
{
    ASSERT_ARG(str);
    int err = 0;
    int c = 0;
    while((c = getchar()) != '\n' && c != EOF) {
        TRY(str_fmt(str, "%c", (char)c), ERR_STR_FMT);  /* append string */
    }
    if(!str->last && (!c || c == EOF || c == '\n')) {
        //THROW("an error"); /* TODO describe this error */
    }
clean:
    fflush(stdin);
    return err;
error: ERR_CLEAN;
} //}}}

void str_remove_trailing_ch(Str *str, char ch, char ch_escape) //{{{
{
    ASSERT_ARG(str);
    while(str_length(str) && str_get_back(str) == ch) {
        if(str_length(str) >= 2) {
            if(str_get_at(str, str_length(str) - 2) == ch_escape) {
                break;
            }
        }
        --str->last;
    }
} //}}}

ErrDecl str_expand_path(Str *path, const Str *base, const Str *home) // TODO: move into platform.c ... {{{
{
    ASSERT_ARG(path);
    ASSERT_ARG(base);
    ASSERT_ARG(home);
    int err = 0;
    Str result = {0};
    Str temp = {0};
    Str base2 = *base;
    str_trim(path);
#if defined(PLATFORM_WINDOWS)
    ABORT("not yet implemented in windows");
#else
    if(!str_length(path)) return 0;
    if(str_length(path) >= 2 && !str_cmp(&STR_IE(*path, 2), &STR("~/"))) {
        TRYC(str_fmt(&result, "%.*s%.*s", STR_F(home), STR_F(&STR_I0(*path, 1))));
        /* assign result */
        str_clear(path);
        temp = *path;
        *path = result;
        result = temp;
    } else if(str_get_front(path) != PLATFORM_CH_SUBDIR) {
        if(!file_is_dir(&base2)) {
            platform_path_up(&base2);
        }
        //printff("%.*s .. %.*s", STR_F(&base2), STR_F(path));
        if(str_length(&base2)) {
            TRYC(str_fmt(&result, "%.*s%c%.*s", STR_F(&base2), PLATFORM_CH_SUBDIR, STR_F(path)));
        } else {
            TRYC(str_fmt(&result, "%.*s", STR_F(path)));
        }
        /* assign result */
        str_clear(path);
        temp = *path;
        *path = result;
        result = temp;
    }
    /* remove any and all dot-dot's -> '..' */
    for(;;) {
        size_t n = str_find_substring(path, &STR(".."));
        if(n >= str_length(path)) break;
        Str prepend = *path;
        Str append = *path;
        prepend.last = prepend.first + n;
        append.first = append.first + n + str_length(&STR(".."));
        str_remove_trailing_ch(&prepend, PLATFORM_CH_SUBDIR, '\\');
        platform_path_up(&prepend);
        str_clear(&result);
        TRYC(str_fmt(&result, "%.*s%.*s", STR_F(&prepend), STR_F(&append)));
        temp = *path;
        *path = result;
        result = temp;
    }
#endif
clean:
    str_free(&temp);
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl str_fmt_line(Str *line, const Str *str, size_t i0, size_t *iE) { //{{{
    ASSERT_ARG(line);
    ASSERT_ARG(str);
    Str fake = *str;
    fake.first += i0;
    size_t i = str_ch(&fake, '\n', 0);
    TRYC(str_fmt(line, "%.*s", (int)i, str_iter_begin(&fake)));
    if(iE) *iE += i + 1; // TODO do I have to/should I check for if i<str_length(str)???
    return 0;
error:
    return -1;
} //}}}

void str_get_line(const Str *str, size_t *i0, size_t *iE) {/*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(i0);
    ASSERT_ARG(iE);
    size_t iE_temp = str_ch(&STR_I0(*str, *i0), '\n', 0) + *i0;
    Str fake_end = STR_IE(*str, iE_temp);
    size_t i0_temp = str_rch(&fake_end, '\n', 0) + 1;
    if(i0_temp > str_length(&fake_end)) {
        *iE = iE_temp;
        *i0 = 0;
    } else {
        *iE = iE_temp;
        *i0 = i0_temp;
    }
    ASSERT(*i0 <= *iE, "expected i0 (%zu) to be smaller than iE (%zu)", *i0, *iE);
}/*}}}*/

ErrDecl str_fmt_fgbg(Str *out, const Str *text, const V3u8 fg, const V3u8 bg, bool bold, bool italic, bool underline) {
    ASSERT_ARG(out);
    ASSERT_ARG(text);
    bool do_fmt = ((fg || bg || bold || italic || underline));
    if(!do_fmt) {
        TRYC(str_fmt(out, "%.*s", STR_F(text)));
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
    if(fg && bg) { TRYC(str_fmt(out, fmt, fg[0], fg[1], fg[2], bg[0], bg[1], bg[2], STR_F(text))); }
    else if(fg) {  TRYC(str_fmt(out, fmt, fg[0], fg[1], fg[2], STR_F(text))); }
    else if(bg) {  TRYC(str_fmt(out, fmt, bg[0], bg[1], bg[2], STR_F(text))); }
    else {         TRYC(str_fmt(out, fmt, STR_F(text))); }
    return 0;
error:
    return -1;
}

// comparing stuff {{{

int str_cmp(const Str *a, const Str *b) //{{{
{
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    size_t la = str_length(a);
    size_t lb = str_length(b);
    if(la != lb) return -1;
    int result = memcmp(str_iter_begin(a), str_iter_begin(b), la);
    return result;
} //}}}

int str_cmp_sortable(const Str *a, const Str *b) //{{{
{
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    //return strcmp(a->s, b->s);
    size_t la = str_length(a);
    size_t lb = str_length(b);
    int result = -1;
    //printff("CMP %zu[%.*s] %zu[%.*s]", str_length(a), STR_F(a), str_length(b), STR_F(b));
    if(la != lb) {
        size_t less = la < lb ? la : lb;
        result = memcmp(str_iter_begin(a), str_iter_begin(b), less);
        if(!result) {
            result = la - lb;
        }
    } else {
        result = memcmp(str_iter_begin(a), str_iter_begin(b), la);
    }
    //printff("%.*s<=>%.*s === %i", STR_F(a), STR_F(b), result);
    return result;
} //}}}

int str_cmp_ci(const Str *a, const Str *b) {/*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    if(str_length(a) != str_length(b)) return -1;
    for (size_t i = 0; i < str_length(a); ++i) {
        int d = tolower(str_get_at(a, i)) - tolower(str_get_at(b, i));
        if (d != 0) return d;
    }
    return 0;
}/*}}}*/

int str_cmp_esci(const Str *a, const Str *b) {/*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
#if 1
    size_t ia = 0, ioff = 0; /* I am abusing this as signed even tho it's unsigned - trust me it'll 100% work! */
    int sa = 0, sb = 0;
    char ca = 0, cb = 0;
    //printf("COMPARE: %.*s[%zu] .. %.*s[%zu]\n", STR_F(a), str_length(a), STR_F(b), str_length(b));
    while(true) {
        /* breaking condition - are we at the end of the string? */
        if(ia >= str_length(a) && ia+ioff >= str_length(b)) break;
        /* get chars at current pos */
        if(ia < str_length(a)) {
            ca = str_get_at(a, ia);
        }
        if(ia+ioff < str_length(b)) {
            cb = str_get_at(b, ia+ioff);
        }
        //printf("ia %zu, ioff %zi, ca '%c', cb '%c'\n", ia, ioff, ca, cb);
        /* check state */
        if(!sa && !sb) {
            if(ca == '\033') ++sa;
            if(cb == '\033') ++sb;
            if(!sa && !sb) {
                /* is one string at the end? */
                if(ia >= str_length(a) && ia+ioff >= str_length(b)) return -1;
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
    if(ia < str_length(a) || (ia+ioff < str_length(b))) {
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
}/*}}}*/

int str_cmp_ci_any(const Str *a, const Str **b, size_t len) {/*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    for (size_t i = 0; i < len; ++i) {
        const Str *bb = b[i];
        int result = str_cmp_ci(a, bb);
        if(!result) return 0;
    }
    return -1;
}/*}}}*/

inline size_t str_count_overlap(const Str *restrict a, const Str *restrict b, bool ignorecase) //{{{
{
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    size_t overlap = 0;
    size_t len = str_length(a) > str_length(b) ? str_length(b) : str_length(a);
    if(!ignorecase) {
        for(size_t i = 0; i < len; ++i) {
            char ca = str_get_at(a, i);
            char cb = str_get_at(b, i);
            if(ca == cb) ++overlap;
            else break;
        }
    } else {
        for(size_t i = 0; i < len; ++i) {
            int ca = tolower(str_get_at(a, i));
            int cb = tolower(str_get_at(b, i));
            if(ca == cb) ++overlap;
            else break;
        }
    }
    return overlap;
} //}}}

inline size_t str_find_substring(const Str *restrict str, const Str *restrict sub) //{{{
{
    ASSERT_ARG(str);
    ASSERT_ARG(sub);
    /* basic checks */
    if(!str_length(sub)) return 0;
    if(str_length(sub) > str_length(str)) {
        return str_length(str);
    }
    /* store original indices */
    Str ref = *str;
    /* check for substring */
    size_t i = 0;
    while(str_length(sub) <= str_length(&ref)) {
        size_t overlap = str_count_overlap(&ref, sub, true);
        if(overlap == str_length(sub)) {
            return i;
        } else {
            i += overlap + 1;
            ref.first += overlap + 1;
        }
    }
    /* restore original */
    return str_length(str);
} //}}}

size_t str_find_any(const Str *str, const Str *any) { //{{{
    ASSERT_ARG(str);
    ASSERT_ARG(any);
    size_t result = str_length(str);
    for(size_t i = 0; i < str_length(any); ++i) {
        size_t temp = str_ch(str, str_get_at(any, i), 0);
        if(temp < result) result = temp;
    }
    return result;
} //}}}

size_t str_find_nany(const Str *str, const Str *any) { //{{{
    ASSERT_ARG(str);
    ASSERT_ARG(any);
    for(size_t i = 0; i < str_length(str); ++i) {
        size_t temp = str_ch(any, str_get_at(str, i), 0);
        if(temp >= str_length(any)) return i;
    }
    return str_length(str);
} //}}}

size_t str_nch(const Str *str, char ch, size_t n) { //{{{
    ASSERT_ARG(str);
    size_t ni = 0;
    for(size_t i = 0; i < str_length(str); ++i) {
        char c = str_get_at(str, i);
        if(c != ch) {
            if(ni == n) return i;
            ++ni;
        }
    }
    return str_length(str);
} //}}}

size_t str_ch(const Str *str, char ch, size_t n) { //{{{
    ASSERT_ARG(str);
    size_t ni = 0;
    for(size_t i = 0; i < str_length(str); ++i) {
        char c = str_get_at(str, i);
        if(c == ch) {
            if(ni == n) return i;
            ++ni;
        }
    }
    return str_length(str);
} //}}}

size_t str_ch_from(const Str *str, char ch, size_t n, size_t from) {/*{{{*/
    ASSERT_ARG(str);
    Str search = STR_LL(str_iter_at(str, from), str_length(str) - from);
    size_t result = str_ch(&search, ch, n) + from;
    return result;
}/*}}}*/

size_t str_ch_pair(const Str *str, char c1) { //{{{
    ASSERT_ARG(str);
    if(!str_length(str)) return str_length(str);
    size_t level = 1;
    char c0 = str_get_at(str, 0);
    for(size_t i = 1; i < str_length(str); ++i) {
        char c = str_get_at(str, i);
        if(c == c0) level++;
        else if(c == c1) level--;
        if(level <= 0) return i;
    }
    return str_length(str);
} //}}}

size_t str_find_ws(const Str *str) { //{{{
    ASSERT_ARG(str);
    for(size_t i = 0; i < str_length(str); ++i) {
        char c = str_get_at(str, i);
        if(isspace(c)) return i;
    }
    return str_length(str);
} //}}}

size_t str_find_nws(const Str *str) { //{{{
    ASSERT_ARG(str);
    for(size_t i = 0; i < str_length(str); ++i) {
        char c = str_get_at(str, i);
        if(!isspace(c)) return i;
    }
    return str_length(str);
} //}}}

// find reverse non-whitespace
size_t str_find_rnws(const Str *str) { //{{{
    ASSERT_ARG(str);
    for(size_t i = str_length(str); i > 0; --i) {
        char c = str_get_at(str, i - 1);
        if(!isspace(c)) return i - 1;
    }
    return str_length(str);
} //}}}

size_t str_rch(const Str *str, char ch, size_t n) //{{{
{
    ASSERT_ARG(str);
    size_t ni = 0;
    for(size_t i = str_length(str); i > 0; --i) {
        char c = str_get_at(str, i - 1);
        if(c == ch) {
            if(ni == n) return i - 1;
            ++ni;
        }
    }
    return str_length(str);
} //}}}

size_t str_rnch(const Str *str, char ch, size_t n) {
    ASSERT_ARG(str);
    size_t ni = 0;
    for(size_t i = str_length(str); i > 0; --i) {
        char c = str_get_at(str, i - 1);
        if(c != ch) {
            if(ni == n) return i - 1;
            ++ni;
        }
    }
    return 0; //str_length(str);
}

size_t str_count_ch(const Str *str, char ch) {/*{{{*/
    ASSERT_ARG(str);
    size_t result = 0;
    if(str->first < str->last) { /* TODO: add this to basically every string utility function :) */
        for(size_t i = 0; i < str_length(str); ++i) {
            char c = str_get_at(str, i);
            if(c == ch) ++result;
        }
    }
    return result;
}/*}}}*/

size_t str_irch(const Str *str, size_t iE, char ch, size_t n) { //{{{
    ASSERT_ARG(str);
    if(iE <= str_length(str)) {
        size_t ni = 0;
        for(size_t i = iE; i > 0; --i) {
            char c = str_get_at(str, i - 1);
            if(c == ch) {
                if(ni == n) return i - 1;
                ++ni;
            }
        }
    }
    return str_length(str);
} //}}}

size_t str_hash(const Str *a) //{{{
{
    ASSERT_ARG(a);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str_length(a)) {
        unsigned char c = (unsigned char)str_get_at(a, i++);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
} //}}}

size_t str_hash_ci(const Str *a) //{{{
{
    ASSERT_ARG(a);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str_length(a)) {
        unsigned char c = (unsigned char)tolower(str_get_at(a, i++));
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
} //}}}

size_t str_hash_esci(const Str *a) {/*{{{*/
    ASSERT_ARG(a);
    size_t hash = 5381;
    size_t i = 0;
    int stage = 0;
    while(i < str_length(a)) {
        unsigned char c = (unsigned char)tolower(str_get_at(a, i++));
        if(!stage) {
            if(c == '\033') ++stage;
            else hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }
        else if(stage == 1 && c == '[') ++stage;
        else if(stage == 2 && c == 'm') stage = 0;
    }
    return hash;
}/*}}}*/

//}}}

Str str_splice(Str *to_splice, Str *prev_splice, char sep) {/*{{{*/
    ASSERT_ARG(to_splice);
    Str result = *to_splice;
    if(prev_splice && prev_splice->s) {
        result.first += str_ch_from(to_splice, sep, 0, str_iter_begin(prev_splice) - str_iter_begin(to_splice)); // TODO is this really += ??
        if((size_t)(str_iter_begin(&result) - str_iter_begin(to_splice)) < str_length(to_splice)) {
            ++result.first;
        }
    }
    result.last = result.first + str_ch(&result, sep, 0);
    return result;
}/*}}}*/

ErrDecl str_remove_escapes(Str *restrict out, Str *restrict in)
{
    ASSERT(out, ERR_NULL_ARG);
    ASSERT(in, ERR_NULL_ARG);
    bool skip = 0;
    size_t iX = 0;
    char c_last = 0;
    for(size_t i = 0; i < str_length(in); i++) {
        char c = str_get_at(in, i);
        if(!skip) {
            if(c == '\033') {
                TRY(str_fmt(out, "%.*s", (int)(i - iX), str_iter_at(in, iX)), ERR_STR_FMT);
                skip = true;
#if 0
            } else if(c == '\'') {
                TRY(str_fmt(out, "%.*s\'\\\'\'", (int)(i - iX), str_iter_at(in, iX)), ERR_STR_FMT);
                iX = i + 1;
#endif
            } else if(!isspace(c_last) && isspace(c)) {
                TRY(str_fmt(out, "%.*s ", (int)(i - iX), str_iter_at(in, iX)), ERR_STR_FMT);
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
        TRY(str_fmt(out, "%.*s", (int)(str_length(in) - iX), str_iter_at(in, iX)), ERR_STR_FMT);
    }
    return 0;
error:
    return -1;
}


