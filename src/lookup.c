#include "str.h"

#include "lookup.h"

size_t int_hash(const int x) { return x * 123456ULL; }
int int_cmp(const int a, const int b) { return a != b; }

size_t int_hash2(const int *x) { return *x * 123456ULL; }
int int_cmp2(const int *a, const int *b) { return *a != *b; }

LUT_IMPLEMENT(TrStr, trstr, Str, BY_REF, void *, BY_VAL, str_hash, str_cmp, 0, 0);

int ttrstritem_cmp(const TTrStrItem *a, const TTrStrItem *b)
{
    return str_cmp_sortable(a->key, b->key);
}

int trstritem_cmp(const TrStrItem *a, const TrStrItem *b)
{
    return str_cmp_sortable(a->key, b->key);
}

int trtrstritem_cmp(const TrTrStrItem *a, const TrTrStrItem *b)
{
    return str_cmp_sortable(a->key, b->key);
}

//LUT_IMPLEMENT(TTrStr, ttrstr, Str, BY_REF, TrStr, BY_REF, str_hash, str_cmp, 0, trstr_free);
LUT_IMPLEMENT(TTrStr, ttrstr, Str, BY_REF, TrStr, BY_REF, str_hash, str_cmp, str_free, trstr_free);
LUT_IMPLEMENT(TrTrStr, trtrstr, Str, BY_REF, TrStr, BY_REF, str_hash, str_cmp, 0, 0);

