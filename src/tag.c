#include "tag.h"

void tag_free(Tag *tag) { //{{{
    vrstr_free(&tag->tags);
    str_free(&tag->filename);
} //}}}

void tagref_free(TagRef *tagref) { //{{{
    vrstr_free(&tagref->filenames);
    str_free(&tagref->tag);
} //}}}

