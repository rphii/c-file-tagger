#ifndef VECTOR_H

#include "vec.h"

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef u8 V3u8[3];

VEC_INCLUDE(VSize, vsize, size_t, BY_VAL, BASE);

#if 0
VEC_INCLUDE(Vu8,  vu8,  uint8_t,  BY_VAL);
VEC_INCLUDE(Vu16, vu16, uint16_t, BY_VAL);
VEC_INCLUDE(Vu32, vu32, uint32_t, BY_VAL);
VEC_INCLUDE(Vu64, vu64, uint64_t, BY_VAL);
#endif

typedef struct TTrStrItem TTrStrItem;
VEC_INCLUDE(VrTTrStrItem, vrttrstritem, TTrStrItem, BY_REF, BASE);
VEC_INCLUDE(VrTTrStrItem, vrttrstritem, TTrStrItem, BY_REF, SORT);

typedef struct TrTrStrItem TrTrStrItem;
VEC_INCLUDE(VrTrTrStrItem, vrtrtrstritem, TrTrStrItem, BY_REF, BASE);
VEC_INCLUDE(VrTrTrStrItem, vrtrtrstritem, TrTrStrItem, BY_REF, SORT);

typedef struct TrStrItem TrStrItem;
VEC_INCLUDE(VrTrStrItem, vrtrstritem, TrStrItem, BY_REF, BASE);
VEC_INCLUDE(VrTrStrItem, vrtrstritem, TrStrItem, BY_REF, SORT);

typedef struct Str Str;
VEC_INCLUDE(VStr, vstr, Str, BY_REF, BASE);
VEC_INCLUDE(VrStr, vrstr, Str, BY_REF, BASE);

typedef struct Slice Slice;
VEC_INCLUDE(VSlice, vslice, Slice, BY_VAL, BASE);

typedef struct Tag Tag;
VEC_INCLUDE(VTag, vtag, Tag, BY_REF, BASE);
VEC_INCLUDE(VrTag, vrtag, Tag, BY_REF, BASE);

typedef struct TagRef TagRef;
VEC_INCLUDE(VrTagRef, vrtagref, TagRef, BY_REF, BASE);

void vrstr_sort(VrStr *vec, size_t *counts);

void vrtag_sort(VrTag *vec);
void vrtagref_sort(VrTagRef *vec, size_t *counts);
//void vrstr_sort(VrStr *vec);

#define VECTOR_H
#endif

