#ifndef LOOKUP_H

#include <rphii/lut.h>

/* other types of lookup tables */

typedef struct RStr RStr;
typedef struct Str Str;
typedef struct Tag Tag;
typedef struct TagRef TagRef;

LUT_INCLUDE(TStr, tstr, Str, BY_REF, void *, BY_VAL);
LUT_INCLUDE(TPStr, tpstr, Str *, BY_VAL, void *, BY_VAL);
LUT_INCLUDE(TTPStr, ttpstr, Str *, BY_VAL, TPStr, BY_REF);
LUT_INCLUDE(RTTPStr, rttpstr, Str *, BY_VAL, TPStr, BY_REF);

//LUT_INCLUDE(TrTag, trtag, Str, BY_REF, TrStr, BY_REF);
//LUT_INCLUDE(TrrTag, trrtag, Str, BY_REF, TrStr, BY_REF);
//LUTS_INCLUDE(TrTagRef, trtagref, TagRef, BY_REF);
//LUTS_INCLUDE(TrrTagRef, trrtagref, TagRef, BY_REF);

#define LOOKUP_H
#endif

