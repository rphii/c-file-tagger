#ifndef SLICE_H

#include <stddef.h>

typedef struct Slice {
    size_t first;
    size_t last;
} Slice;

typedef struct Str Str;

Str slice_str(Slice slice, Str *str);
Str slice_cstr(Slice slice, char *str);

#define SLICE_H
#endif

