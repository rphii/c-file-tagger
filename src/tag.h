#ifndef TAG_H

#include "slice.h"
#include "vector.h"
#include "str.h"
#include "lookup.h"

typedef struct Tag {
    Str filename;
    VrStr tags;
} Tag;

typedef struct TagRef {
    VrStr filenames;
    Str tag;
} TagRef;

void tag_free(Tag *tag);
void tagref_free(TagRef *tagref);

#define TAG_H
#endif

