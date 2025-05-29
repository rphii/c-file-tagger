#include <rphii/str.h>

#include "vector.h"
#include "lookup.h"

int ttrstrkv_cmp(const RTTPStrKV *a, const RTTPStrKV *b)
{
    return str_pcmp_sortable(a->key, b->key);
}

int tpstrkv_cmp(const TPStrKV *a, const TPStrKV *b)
{
    return str_pcmp_sortable(a->key, b->key);
}

VEC_IMPLEMENT(VRTTPStrKV, vrttpstrkv, RTTPStrKV, BY_REF, BASE, 0);
VEC_IMPLEMENT(VRTTPStrKV, vrttpstrkv, RTTPStrKV, BY_REF, SORT, ttrstrkv_cmp);

VEC_IMPLEMENT(VRTPStrKV, vrtpstrkv, TPStrKV, BY_REF, BASE, 0);
VEC_IMPLEMENT(VRTPStrKV, vrtpstrkv, TPStrKV, BY_REF, SORT, tpstrkv_cmp);

#if 0
VEC_IMPLEMENT(VrTTrStrItem, vrttrstritem, TTrStrItem, BY_REF, BASE, 0);
VEC_IMPLEMENT(VrTTrStrItem, vrttrstritem, TTrStrItem, BY_REF, SORT, ttrstritem_cmp);

VEC_IMPLEMENT(VrTrStrItem, vrtrstritem, TrStrItem, BY_REF, BASE, 0);
VEC_IMPLEMENT(VrTrStrItem, vrtrstritem, TrStrItem, BY_REF, SORT, trstritem_cmp);

VEC_IMPLEMENT(VrTrTrStrItem, vrtrtrstritem, TrTrStrItem, BY_REF, BASE, 0);
VEC_IMPLEMENT(VrTrTrStrItem, vrtrtrstritem, TrTrStrItem, BY_REF, SORT, trtrstritem_cmp);
#endif

#if 0
void vrttrstritem_sort(VrTTrStrItem *vec)
{
    /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */
    size_t h, i, j, n = vrttrstritem_length(vec);
    TTrStrItem temp;
    for (h = n; h /= 2;) {
        for (i = h; i < n; i++) {
            //t = a[i];
            temp = *vrttrstritem_get_at(vec, i);
            //for (j = i; j >= h && t < a[j - h]; j -= h) {
            for (j = i; j >= h && str_cmp_sortable(temp.key, vrttrstritem_get_at(vec, j-h)->key) < 0; j -= h) {
                vrttrstritem_set_at(vec, j, vrttrstritem_get_at(vec, j-h));
                //a[j] = a[j - h];
            }
            //a[j] = t;
            vrttrstritem_set_at(vec, j, &temp);
        }
    }
}

void vrtrstritem_sort(VrTrStrItem *vec)
{
    /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */
    size_t h, i, j, n = vrtrstritem_length(vec);
    TrStrItem temp;
    for (h = n; h /= 2;) {
        for (i = h; i < n; i++) {
            //t = a[i];
            temp = *vrtrstritem_get_at(vec, i);
            //for (j = i; j >= h && t < a[j - h]; j -= h) {
            for (j = i; j >= h && str_cmp_sortable(temp.key, vrtrstritem_get_at(vec, j-h)->key) < 0; j -= h) {
                vrtrstritem_set_at(vec, j, vrtrstritem_get_at(vec, j-h));
                //a[j] = a[j - h];
            }
            //a[j] = t;
            vrtrstritem_set_at(vec, j, &temp);
        }
    }
}
#endif



//VEC_IMPLEMENT(VStr, vstr, Str, BY_REF, BASE, str_free);
//VEC_IMPLEMENT(VStr, vstr, Str, BY_REF, ERR);
//VEC_IMPLEMENT(VrStr, vrstr, Str, BY_REF, BASE, 0);
//VEC_IMPLEMENT(VrStr, vrstr, Str, BY_REF, SORT, str_cmp_sortable);
//VEC_IMPLEMENT(VrStr, vrstr, Str, BY_REF, ERR);

#if 0
void vrstr_sort(VrStr *vec, size_t *counts)
{
    /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */
    size_t h, i, j, n = vrstr_length(*vec);
    Str temp;
    size_t temp2 = 0;
    for (h = n; h /= 2;) {
        for (i = h; i < n; i++) {
            //t = a[i];
            temp = *vrstr_get_at(vec, i);
            if(counts) temp2 = counts[i];
            //for (j = i; j >= h && t < a[j - h]; j -= h) {
            for (j = i; j >= h && str_cmp_sortable(&temp, vrstr_get_at(vec, j-h)) < 0; j -= h) {
                vrstr_set_at(vec, j, vrstr_get_at(vec, j-h));
                if(counts) counts[j] = counts[j-h];
                //a[j] = a[j - h];
            }
            //a[j] = t;
            vrstr_set_at(vec, j, &temp);
            if(counts) counts[j] = temp2;
        }
    }
}
#endif



