#include "str.h"
#include "slice.h"
#include "tag.h"

#include "vector.h"

VEC_IMPLEMENT(VStr, vstr, Str, BY_REF, str_free);
VEC_IMPLEMENT(VrStr, vrstr, Str, BY_REF, 0);

VEC_IMPLEMENT(VSlice, vslice, Slice, BY_VAL, 0);

VEC_IMPLEMENT(VTag, vtag, Tag, BY_REF, tag_free);
VEC_IMPLEMENT(VrTag, vrtag, Tag, BY_REF, 0);
VEC_IMPLEMENT(VrTagRef, vrtagref, TagRef, BY_REF, 0);

void vrtag_sort(VrTag *vec)
{
    /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */
    size_t h, i, j, n = vrtag_length(vec);
    Tag temp;
    for (h = n; h /= 2;) {
        for (i = h; i < n; i++) {
            //t = a[i];
            temp = *vrtag_get_at(vec, i);
            //for (j = i; j >= h && t < a[j - h]; j -= h) {
            for (j = i; j >= h && str_cmp(&temp.filename, &vrtag_get_at(vec, j-h)->filename) < 0; j -= h) {
                vrtag_set_at(vec, j, vrtag_get_at(vec, j-h));
                //a[j] = a[j - h];
            }
            //a[j] = t;
            vrtag_set_at(vec, j, &temp);
        }
    }
}

void vrtagref_sort(VrTagRef *vec, size_t *counts)
{
    /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */
    size_t h, i, j, n = vrtagref_length(vec);
    TagRef temp;
    size_t temp2 = 0;
    for (h = n; h /= 2;) {
        for (i = h; i < n; i++) {
            //t = a[i];
            temp = *vrtagref_get_at(vec, i);
            if(counts) temp2 = counts[i];
            //for (j = i; j >= h && t < a[j - h]; j -= h) {
            for (j = i; j >= h && str_cmp(&temp.tag, &vrtagref_get_at(vec, j-h)->tag) < 0; j -= h) {
                vrtagref_set_at(vec, j, vrtagref_get_at(vec, j-h));
                if(counts) counts[j] = counts[j-h];
                //a[j] = a[j - h];
            }
            //a[j] = t;
            vrtagref_set_at(vec, j, &temp);
            if(counts) counts[j] = temp2;
        }
    }
}

void vrstr_sort(VrStr *vec)
{
    /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */
#if 1
    size_t h, i, j, n = vrstr_length(vec);
    Str temp;
    for (h = n; h /= 2;) {
        for (i = h; i < n; i++) {
            //t = a[i];
            temp = *vrstr_get_at(vec, i);
            //for (j = i; j >= h && t < a[j - h]; j -= h) {
            for (j = i; j >= h && str_cmp(&temp, vrstr_get_at(vec, j-h)) < 0; j -= h) {
                vrstr_set_at(vec, j, vrstr_get_at(vec, j-h));
                //a[j] = a[j - h];
            }
            //a[j] = t;
            vrstr_set_at(vec, j, &temp);
        }
    }
#endif
}


