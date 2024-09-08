#ifndef LOOKUP_H

#include "lut.h"

/* other types of lookup tables */

typedef struct Str Str;
typedef struct Tag Tag;
typedef struct TagRef TagRef;

LUT_INCLUDE(TrStr, trstr, Str, BY_REF, bool, BY_VAL);

//LUT_INCLUDE(TrTag, trtag, Str, BY_REF, TrStr, BY_REF);
//LUT_INCLUDE(TrrTag, trrtag, Str, BY_REF, TrStr, BY_REF);
//LUTS_INCLUDE(TrTagRef, trtagref, TagRef, BY_REF);
//LUTS_INCLUDE(TrrTagRef, trrtagref, TagRef, BY_REF);

#define LOOKUP_H
#endif

