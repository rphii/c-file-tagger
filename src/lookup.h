#ifndef LOOKUP_H

#include "lutd.h"

/* other types of lookup tables */

typedef struct Str Str;
typedef struct Tag Tag;
typedef struct TagRef TagRef;

LUTD_INCLUDE(TrStr, trstr, Str, BY_REF);

LUTD_INCLUDE(TrTag, trtag, Tag, BY_REF);
LUTD_INCLUDE(TrrTag, trrtag, Tag, BY_REF);
LUTD_INCLUDE(TrTagRef, trtagref, TagRef, BY_REF);
LUTD_INCLUDE(TrrTagRef, trrtagref, TagRef, BY_REF);

#define LOOKUP_H
#endif

