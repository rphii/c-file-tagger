#include "str.h"

#include "lookup.h"

size_t int_hash(const int x) { return x * 123456ULL; }
int int_cmp(const int a, const int b) { return a != b; }

size_t int_hash2(const int *x) { return *x * 123456ULL; }
int int_cmp2(const int *a, const int *b) { return *a != *b; }

#if 0
LUT_IMPLEMENT(TrStr, trstr, Str, BY_REF, void *, BY_VAL, str_phash, str_pcmp, 0, 0);

int ttrstritem_cmp(const TTrStrItem *a, const TTrStrItem *b)
{
    return str_cmp_sortable(a->key, b->key);
}
#endif

#if 0
int trstritem_cmp(const TrStrItem *a, const TrStrItem *b)
{
    return rstr_cmp_sortable(a->key, b->key);
}
#endif

#if 0
int trtrstritem_cmp(const TrTrStrItem *a, const TrTrStrItem *b)
{
    return str_cmp_sortable(a->key, b->key);
}

//LUT_IMPLEMENT(TTrStr, ttrstr, Str, BY_REF, TrStr, BY_REF, str_hash, str_cmp, 0, trstr_free);
LUT_IMPLEMENT(TTrStr, ttrstr, Str, BY_REF, TrStr, BY_REF, str_phash, str_pcmp, str_free, trstr_free);
LUT_IMPLEMENT(TrTrStr, trtrstr, Str, BY_REF, TrStr, BY_REF, str_phash, str_pcmp, 0, 0);
#endif

LUT_IMPLEMENT(TStr, tstr, Str, BY_REF, void *, BY_VAL, str_phash, str_pcmp, str_free, 0);
LUT_IMPLEMENT(TPStr, tpstr, Str *, BY_VAL, void *, BY_VAL, str_phash, str_pcmp, 0, 0);
LUT_IMPLEMENT(TTPStr, ttpstr, Str *, BY_VAL, TPStr, BY_REF, str_phash, str_pcmp, 0, tpstr_free);
LUT_IMPLEMENT(RTTPStr, rttpstr, Str *, BY_VAL, TPStr, BY_REF, str_phash, str_pcmp, 0, 0);


