#include "tag.h"

#include "lookup.h"

size_t trtag_hash(Tag *tag) {
    size_t result = str_hash(&tag->filename);
    //printff("tag:hash:%.*s=%zu", STR_F(&tag->filename), result);
    return result;
};

int trtag_cmp(Tag *a, Tag *b) {
    int result = str_cmp(&a->filename, &b->filename);
    //printff("tag:cmp:%.*s<=>%.*s=%u", STR_F(&a->filename), STR_F(&b->filename), result);
    return result;
};

size_t ttag_hash(Tag tag) {
    size_t result = str_hash(&tag.filename);
    //printff("tag:hash:%.*s=%zu", STR_F(&tag->filename), result);
    return result;
};

int ttag_cmp(Tag a, Tag b) {
    int result = str_cmp(&a.filename, &b.filename);
    //printff("tag:cmp:%.*s<=>%.*s=%u", STR_F(&a->filename), STR_F(&b->filename), result);
    return result;
};

LUTD_IMPLEMENT(TTag, ttag, Tag, BY_VAL, ttag_hash, ttag_cmp, 0);

LUTD_IMPLEMENT(TrTag, trtag, Tag, BY_REF, trtag_hash, trtag_cmp, tag_free);
LUTD_IMPLEMENT(TrrTag, trrtag, Tag, BY_REF, trtag_hash, trtag_cmp, 0);
//LUTD_IMPLEMENT(TrrTagRef, trrtagref, Tag, BY_REF, trtag_hash, trtag_cmp, 0);

size_t trtagref_hash(TagRef *tag) {
    size_t result = str_hash(&tag->tag);
    //printff("tagref:hash:%.*s=%zu", STR_F(&tag->tag), result);
    return result;
};

int trtagref_cmp(TagRef *a, TagRef *b) {
    int result = str_cmp(&a->tag, &b->tag);
    //printff("tagref:cmp:%.*s<=>%.*s=%u", STR_F(&a->tag), STR_F(&b->tag), result);
    return result;
};

LUTD_IMPLEMENT(TrTagRef, trtagref, TagRef, BY_REF, trtagref_hash, trtagref_cmp, tagref_free);
LUTD_IMPLEMENT(TrrTagRef, trrtagref, TagRef, BY_REF, trtagref_hash, trtagref_cmp, 0);

