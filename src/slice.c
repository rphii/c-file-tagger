#include "slice.h"
#include "str.h"

Str slice_str(Slice slice, Str *str) { //{{{
    Str result = { .first = slice.first, .last = slice.last, .s = str->s };
    return result;
} //}}}

Str slice_cstr(Slice slice, char *str) { //{{{
    Str result = { .first = slice.first, .last = slice.last, .s = str };
    return result;
} //}}}



