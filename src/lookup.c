#include "tag.h"
#include "str.h"

#include "lookup.h"

LUTD_IMPLEMENT(TrStr, trstr, Str, BY_REF, str_hash, str_cmp, 0);

size_t trtag_hash(const Tag *tag) {
    size_t result = str_hash(&tag->filename);
    //printff("tag:hash:%.*s=%zu", STR_F(&tag->filename), result);
    return result;
};

int trtag_cmp(const Tag *a, const Tag *b) {
    int result = str_cmp(&a->filename, &b->filename);
    //printff("tag:cmp:%.*s<=>%.*s=%u", STR_F(&a->filename), STR_F(&b->filename), result);
    return result;
};

LUTD_IMPLEMENT(TrTag, trtag, Tag, BY_REF, trtag_hash, trtag_cmp, tag_free);
LUTD_IMPLEMENT(TrrTag, trrtag, Tag, BY_REF, trtag_hash, trtag_cmp, 0);
//LUTD_IMPLEMENT(TrrTagRef, trrtagref, Tag, BY_REF, trtag_hash, trtag_cmp, 0);

size_t trtagref_hash(const TagRef *tag) {
    size_t result = str_hash(&tag->tag);
    //printff("tagref:hash:%.*s=%zu", STR_F(&tag->tag), result);
    return result;
};

int trtagref_cmp(const TagRef *a, const TagRef *b) {
    int result = str_cmp(&a->tag, &b->tag);
    //printff("tagref:cmp:%.*s<=>%.*s=%u", STR_F(&a->tag), STR_F(&b->tag), result);
    return result;
};

LUTD_IMPLEMENT(TrTagRef, trtagref, TagRef, BY_REF, trtagref_hash, trtagref_cmp, tagref_free);
LUTD_IMPLEMENT(TrrTagRef, trrtagref, TagRef, BY_REF, trtagref_hash, trtagref_cmp, 0);

