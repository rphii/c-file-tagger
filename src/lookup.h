#ifndef LOOKUP_H

#include <rlc/lut.h>
#include <rlso.h>

/* other types of lookup tables */

LUT_INCLUDE(TStr, tstr, So, BY_REF, void *, BY_VAL);
LUT_INCLUDE(TPStr, tpstr, So *, BY_VAL, void *, BY_VAL);
LUT_INCLUDE(TTPStr, ttpstr, So *, BY_VAL, TPStr, BY_REF);
LUT_INCLUDE(RTTPStr, rttpstr, So *, BY_VAL, TPStr, BY_REF);

//LUT_INCLUDE(TrTag, trtag, Str, BY_REF, TrStr, BY_REF);
//LUT_INCLUDE(TrrTag, trrtag, Str, BY_REF, TrStr, BY_REF);
//LUTS_INCLUDE(TrTagRef, trtagref, TagRef, BY_REF);
//LUTS_INCLUDE(TrrTagRef, trrtagref, TagRef, BY_REF);

#define LOOKUP_H
#endif

