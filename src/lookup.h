#ifndef LOOKUP_H

#include "lut.h"

/* other types of lookup tables */

typedef struct Str Str;
typedef struct Tag Tag;
typedef struct TagRef TagRef;

LUT_INCLUDE(TrStr, trstr, Str, BY_REF, void *, BY_VAL);
LUT_INCLUDE(TTrStr, ttrstr, Str, BY_REF, TrStr, BY_REF);
LUT_INCLUDE(TrTrStr, trtrstr, Str, BY_REF, TrStr, BY_REF);

int trtrstritem_cmp(const TrTrStrItem *a, const TrTrStrItem *b);
int ttrstritem_cmp(const TTrStrItem *a, const TTrStrItem *b);
int trstritem_cmp(const TrStrItem *a, const TrStrItem *b);

//LUT_INCLUDE(TrTag, trtag, Str, BY_REF, TrStr, BY_REF);
//LUT_INCLUDE(TrrTag, trrtag, Str, BY_REF, TrStr, BY_REF);
//LUTS_INCLUDE(TrTagRef, trtagref, TagRef, BY_REF);
//LUTS_INCLUDE(TrrTagRef, trrtagref, TagRef, BY_REF);

#define LOOKUP_H
#endif

