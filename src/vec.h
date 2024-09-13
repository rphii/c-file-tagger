/* MIT License

Copyright (c) 2023 rphii

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

//#ifndef VEC_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <stdio.h>

#include "err.h"

#define VEC_PLATFORM    1

#if VEC_PLATFORM
#include "platform_detect.h"
#define VEC_TRACE   platform_trace()
#else
#define VEC_TRACE   do {} while(0)
#endif

#ifndef vec_malloc
#define vec_malloc  malloc
#endif
#ifndef vec_realloc
#define vec_realloc realloc
#endif
#ifndef vec_memset
#define vec_memset  memset
#endif
#ifndef vec_memcpy
#define vec_memcpy  memcpy
#endif
#ifndef vec_memmove
#define vec_memmove memmove
#endif

#undef  VEC_DEFAULT_SIZE
#ifdef  VEC_SETTINGS_DEFAULT_SIZE
#define VEC_DEFAULT_SIZE VEC_SETTINGS_DEFAULT_SIZE
#else
#define VEC_DEFAULT_SIZE    4
#endif

#undef  VEC_STRUCT_ITEMS
#ifdef  VEC_SETTINGS_STRUCT_ITEMS
#define VEC_STRUCT_ITEMS VEC_SETTINGS_STRUCT_ITEMS
#else
#define VEC_STRUCT_ITEMS items
#endif

#undef  VEC_KEEP_ZERO_END
#ifdef  VEC_SETTINGS_KEEP_ZERO_END
#define VEC_KEEP_ZERO_END VEC_SETTINGS_KEEP_ZERO_END
#else
#define VEC_KEEP_ZERO_END 0
#endif

#define ERR_VEC_PUSH_BACK "failed pushing value to vector"
#define ERR_VEC_RESIZE "failed resizing vector"
#define ERR_VEC_RESERVE "failed reserving memory in vector"

#ifndef VEC_H

#define VEC_CAST_FREE(X)        ((void *)(X))
#define VEC_TYPE_FREE(F,X,T)    ((void (*)(T *))(F))(VEC_CAST_FREE(X))

typedef enum
{
    VEC_ERROR_NONE,
    /* errors below */
    VEC_ERROR_MALLOC,
    VEC_ERROR_REALLOC,
    /* errors above */
    VEC_ERROR__COUNT,
} VecErrorList;

#define VEC_ITEM_BY_VAL(T)  T
#define VEC_ITEM_BY_REF(T)  T *
#define VEC_ITEM(T, M)  VEC_ITEM_##M(T)

#define VEC_REF_BY_VAL   &
#define VEC_REF_BY_REF
#define VEC_REF(M)       VEC_REF_##M

#define VEC_ASSERT_REAL(x)   ASSERT(x, "assertion failed")

#define VEC_ASSERT_BY_REF(x)    VEC_ASSERT_REAL(x)
#define VEC_ASSERT_BY_VAL(x)
#define VEC_ASSERT(x, M)        VEC_ASSERT_##M(x)

#define VEC_IS_BY_REF_BY_REF 1
#define VEC_IS_BY_REF_BY_VAL 0
#define VEC_IS_BY_REF(M)     VEC_IS_BY_REF_##M

#define VEC_H
#endif

/*
 * === THE VECTOR DESCRIPTION ===
 * N = name - used for the type name
 * A = abbreviation - used for the functions
 * T = type - the actual type to base the vector off of
 * F = free - function to use if your structure itself has a custom free routine
 * M = mode - either BY_VAL or BY_REF
 */

#define VEC_INCLUDE(N, A, T, M, MODULE, ...) \
    VEC_INCLUDE_##MODULE(N, A, T, M, ##__VA_ARGS__)

#define VEC_INCLUDE_BASE(N, A, T, M) \
    typedef struct N { \
        size_t cap; \
        size_t last; \
        size_t first; \
        VEC_ITEM(T, M) *VEC_STRUCT_ITEMS; \
    } N; \
    \
    /* common implementation */ \
    void A##_clear(N *vec); \
    size_t A##_length(const N *vec); \
    size_t A##_capacity(const N *vec); \
    int A##_empty(const N *vec); \
    int A##_resize(N *vec, size_t cap); \
    int A##_shrink(N *vec); \
    /* single item operations */ \
    int A##_push_front(N *vec, VEC_ITEM(T, M) val); \
    int A##_push_back(N *vec, VEC_ITEM(T, M) val); \
    int A##_push_at(N *vec, size_t index, VEC_ITEM(T, M) val); \
    /* TODO */ void A##_set_front(N *vec, VEC_ITEM(T, M) val); \
    /* TODO */ void A##_set_back(N *vec, VEC_ITEM(T, M) val); \
    void A##_set_at(N *vec, size_t index, VEC_ITEM(T, M) val); \
    void A##_pop_front(N *vec, T *val); \
    void A##_pop_back(N *vec, T *val); \
    void A##_pop_at(N *vec, size_t index, T *val); \
    VEC_ITEM(T, M) A##_get_front(const N *vec); \
    VEC_ITEM(T, M) A##_get_back(const N *vec); \
    VEC_ITEM(T, M) A##_get_at(const N *vec, size_t index); \
    /* slice operations */ \
    /* TODO */ int A##_extend_front(N *vec, N *v2); \
    /* TODO */ int A##_extend_back(N *vec, N *v2); \
    /* TODO */ int A##_extend_at(N *vec, size_t index, N *v2); \
    /* TODO */ int A##_paste_front(N *vec, N *v2, size_t n); \
    /* TODO */ int A##_paste_back(N *vec, N *v2, size_t n); \
    /* TODO */ int A##_paste_at(N *vec, size_t index, N *v2, size_t n); \
    /* TODO */ int A##_cut_front(N *vec, N *v2, size_t n); \
    /* TODO */ int A##_cut_back(N *vec, N *v2, size_t n); \
    /* TODO */ int A##_cut_at(N *vec, size_t from, size_t n, N *v2); \
    /* TODO */ int A##_cat_front(N *vec, N *v2, size_t n); \
    /* TODO */ int A##_cat_back(N *vec, N *v2, size_t n); \
    /* TODO */ int A##_cat_at(N *vec, size_t from, size_t n, N *v2); \
    /* miscellaneous operations */ \
    void A##_swap(N *vec, size_t i1, size_t i2); \
    void A##_reverse(N *vec); \
    /* TODO */ void A##_reverse_slice(N *vec, size_t from, size_t n); \
    VEC_ITEM(T, M)*A##_iter_begin(const N *vec); \
    VEC_ITEM(T, M)*A##_iter_end(const N *vec); \
    VEC_ITEM(T, M)*A##_iter_at(const N *vec, size_t index); \
    /* split implementation */ \
    void A##_free(N *vec); \
    void A##_zero(N *vec); \
    size_t A##_reserved(const N *vec); \
    int A##_reserve(N *vec, size_t cap); \
    int A##_copy(N *dst, const N *src); \

#define VEC_INCLUDE_SORT(N, A, T, M)                void A##_sort(N *vec);

/*
 * int A##_cmp(N *a, N *b) -> compare vec
 * ssize_t A##_find(N *vec, ssize_t index) -> find item
 * ssize_t A##_match(N *vec,  -> find vec
 */

/**********************************************************/
/* IMPLEMENTATION *****************************************/
/**********************************************************/

#define VEC_IMPLEMENT(N, A, T, M, MODULE, ...) \
    VEC_IMPLEMENT_##MODULE(N, A, T, M, ##__VA_ARGS__) \

#define VEC_IMPLEMENT_BASE(N, A, T, M, F) \
    /* private */ \
    /*VEC_IMPLEMENT_COMMON_STATIC_F(N, A, T, F);              */\
    VEC_IMPLEMENT_COMMON_STATIC_ZERO(N, A, T, F);           \
    VEC_IMPLEMENT_COMMON_STATIC_GET(N, A, T, F, M);         \
    VEC_IMPLEMENT_##M##_STATIC_SHRINK_BACK(N, A, T, F);     \
    VEC_IMPLEMENT_##M##_STATIC_SHRINK_FRONT(N, A, T, F);    \
    /* public */ \
    VEC_IMPLEMENT_COMMON_CLEAR(N, A, T, F);             \
    VEC_IMPLEMENT_COMMON_LENGTH(N, A, T, F);            \
    VEC_IMPLEMENT_COMMON_CAPACITY(N, A, T, F);          \
    VEC_IMPLEMENT_COMMON_EMPTY(N, A, T, F);             \
    VEC_IMPLEMENT_COMMON_RESIZE(N, A, T, F);            \
    VEC_IMPLEMENT_COMMON_SHRINK(N, A, T, F);            \
    VEC_IMPLEMENT_COMMON_ITER_BEGIN(N, A, T, F, M);     \
    VEC_IMPLEMENT_COMMON_ITER_END(N, A, T, F, M);       \
    VEC_IMPLEMENT_COMMON_ITER_AT(N, A, T, F, M);        \
    VEC_IMPLEMENT_COMMON_POP_FRONT(N, A, T, F, M);      \
    VEC_IMPLEMENT_COMMON_POP_BACK(N, A, T, F, M);       \
    VEC_IMPLEMENT_COMMON_POP_AT(N, A, T, F, M);         \
    VEC_IMPLEMENT_COMMON_GET_AT(N, A, T, F, M);         \
    VEC_IMPLEMENT_COMMON_GET_FRONT(N, A, T, F, M);      \
    VEC_IMPLEMENT_COMMON_GET_BACK(N, A, T, F, M);       \
    VEC_IMPLEMENT_COMMON_SET_AT(N, A, T, F, M);         \
    VEC_IMPLEMENT_COMMON_PUSH_FRONT(N, A, T, F, M);     \
    VEC_IMPLEMENT_COMMON_PUSH_BACK(N, A, T, F, M);      \
    VEC_IMPLEMENT_COMMON_PUSH_AT(N, A, T, F, M);        \
    /* slice */ \
    VEC_IMPLEMENT_COMMON_EXTEND_FRONT(N, A, T, F, M);   \
    VEC_IMPLEMENT_COMMON_EXTEND_AT(N, A, T, F, M);      \
    VEC_IMPLEMENT_COMMON_CUT_FRONT(N, A, T, F, M);      \
    VEC_IMPLEMENT_COMMON_CUT_BACK(N, A, T, F, M);       \
    VEC_IMPLEMENT_COMMON_CUT_AT(N, A, T, F, M);         \
    VEC_IMPLEMENT_COMMON_PASTE_FRONT(N, A, T, F, M);    \
    VEC_IMPLEMENT_COMMON_PASTE_BACK(N, A, T, F, M);     \
    VEC_IMPLEMENT_COMMON_PASTE_AT(N, A, T, F, M);       \
    VEC_IMPLEMENT_COMMON_CAT_FRONT(N, A, T, F, M);      \
    VEC_IMPLEMENT_COMMON_CAT_BACK(N, A, T, F, M);       \
    VEC_IMPLEMENT_COMMON_CAT_AT(N, A, T, F, M);         \
    /* rest */ \
    VEC_IMPLEMENT_COMMON_SWAP(N, A, T, F, M);           \
    VEC_IMPLEMENT_COMMON_REVERSE(N, A, T, F);           \
    VEC_IMPLEMENT_##M##_EXTEND_BACK(N, A, T, F, M);    \
    VEC_IMPLEMENT_##M##_FREE(N, A, T, F);               \
    VEC_IMPLEMENT_##M##_ZERO(N, A, T, F);               \
    VEC_IMPLEMENT_##M##_RESERVED(N, A, T, F);           \
    VEC_IMPLEMENT_##M##_RESERVE(N, A, T, F);            \
    VEC_IMPLEMENT_##M##_COPY(N, A, T, F);               \

/**********************************************************/
/* PRIVATE FUNCTION IMPLEMENTATIONS ***********************/
/**********************************************************/

/* implementation for both */

/**
 * @brief A##_static_f [COMMON] - internal use, sets up the optional freeing function
 */
/*#define VEC_IMPLEMENT_COMMON_STATIC_F(N, A, T, F) \
    static void (*A##_static_f)(void *) = F != 0 ? VEC_TYPE_FREE(F) : 0; \
    */


/* implementation by val */

/**
 * @brief A##_static_get [COMMON] - internal use, get reference to item at index
 * @param vec - the vector
 * @param index - the index
 * @return pointer to item (by reference) at index
 */
#define VEC_IMPLEMENT_COMMON_STATIC_GET(N, A, T, F, M) \
    static inline VEC_ITEM(T, M) *A##_static_get(const N *vec, size_t index) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(index < vec->last); \
        VEC_ASSERT_REAL(index >= vec->first); \
        return &vec->VEC_STRUCT_ITEMS[index]; \
    }

/**
 * @brief A##_static_shrink_back [BY_VAL] - internal use, shrink end of vector (resize)
 * @param vec - the vector
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_VAL_STATIC_SHRINK_BACK(N, A, T, F) \
    static inline int A##_static_shrink_back(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t cap = vec->cap; \
        size_t last = vec->last; \
        size_t required = vec->cap ? vec->cap : VEC_DEFAULT_SIZE;\
        while(required > last) required /= 2; \
        required *= 2; \
        if(required  < vec->cap) { \
            if(required) { \
                if(F != 0) { \
                    for(size_t i = required; i < cap; i++) { \
                        VEC_TYPE_FREE(F, &vec->VEC_STRUCT_ITEMS[i], T); \
                    } \
                } \
                void *temp = vec_realloc(vec->VEC_STRUCT_ITEMS, sizeof(*vec->VEC_STRUCT_ITEMS) * required); \
                if(!temp) return VEC_ERROR_REALLOC; \
                vec->VEC_STRUCT_ITEMS = temp; \
                vec->cap = required; \
            } else { \
                A##_free(vec); \
            } \
        } \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_static_shrink_front [BY_VAL] - internal use, shrink vector at beginning (resize)
 * @param vec - the vector
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_VAL_STATIC_SHRINK_FRONT(N, A, T, F) \
    static inline int A##_static_shrink_front(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t first = vec->first; \
        if(first) { \
            vec->first = 0; \
            T *item = A##_static_get(vec, 0); \
            if(F != 0) { \
                for(size_t i = 0; i < first; i++) { \
                    VEC_TYPE_FREE(F, &vec->VEC_STRUCT_ITEMS[i], T); \
                } \
            } \
            vec_memmove(item, item + first, sizeof(T) * (vec->last - first)); \
            /* TODO: is that really needed? */ vec_memset(item + vec->last - first, 0, sizeof(T) * (first)); \
            vec->last -= first; \
        } \
        return 0; \
    }

/* implementation by ref */

/**
 * @brief A##_static_shrink_back [BY_REF] - internal use, shrink end of vector (resize)
 * @param vec - the vector
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_REF_STATIC_SHRINK_BACK(N, A, T, F) \
    static inline int A##_static_shrink_back(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t cap = vec->cap; \
        size_t last = vec->last; \
        size_t required = vec->cap ? vec->cap : VEC_DEFAULT_SIZE;\
        while(required > last) required /= 2; \
        required *= 2; \
        if(required < vec->cap) { \
            if(required) { \
                for(size_t i = required; i < cap; i++) { \
                    if(F != 0) { \
                        VEC_TYPE_FREE(F, vec->VEC_STRUCT_ITEMS[i], T); \
                    } \
                    free(vec->VEC_STRUCT_ITEMS[i]); \
                    /*memset(&vec->VEC_STRUCT_ITEMS[i], 0, sizeof(vec->VEC_STRUCT_ITEMS[i]));*/ \
                } \
                void *temp = vec_realloc(vec->VEC_STRUCT_ITEMS, sizeof(*vec->VEC_STRUCT_ITEMS) * required); \
                if(!temp) return VEC_ERROR_REALLOC; \
                vec->VEC_STRUCT_ITEMS = temp; \
                vec->cap = required; \
            } else { \
                A##_free(vec); \
            } \
        } \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_static_shrink_front [BY_REF] - internal use, shrink vector at beginning (resize)
 * @param vec - the vector
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_REF_STATIC_SHRINK_FRONT(N, A, T, F) \
    static inline int A##_static_shrink_front(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t first = vec->first; \
        if(first) { \
            vec->first = 0; \
            T **item = A##_static_get(vec, 0); \
            if(F != 0) { \
                for(size_t i = 0; i < first; i++) { \
                    VEC_TYPE_FREE(F, vec->VEC_STRUCT_ITEMS[i], T); \
                } \
            } \
            /* we have to preserve all items, since they're alloced */ \
            T **residuals = malloc(sizeof(T *) * first); \
            if(!residuals) return VEC_ERROR_MALLOC; \
            vec_memcpy(residuals, item, sizeof(T *) * (first)); \
            vec_memmove(item, item + first, sizeof(T *) * (vec->last - first)); \
            vec_memcpy(item + vec->last - first, residuals, sizeof(T *) * (first)); \
            free(residuals); \
            vec->last -= first; \
        } \
        return 0; \
    }


/**********************************************************/
/* PUBLIC FUNCTION IMPLEMENTATIONS ************************/
/**********************************************************/

/* implementation for both */

#define VEC_IMPLEMENT_SORT(N, A, T, M, CMP) \
    void A##_sort(N *vec) { \
        /* shell sort, https://rosettacode.org/wiki/Sorting_algorithms/Shell_sort#C */ \
        size_t h, i, j, n = A##_length(vec); \
        T temp; \
        for (h = n; h /= 2;) { \
            for (i = h; i < n; i++) { \
                /*t = a[i]; */\
                temp = *A##_get_at(vec, i); \
                /*for (j = i; j >= h && t < a[j - h]; j -= h) { */\
                for (j = i; j >= h && CMP(&temp, A##_get_at(vec, j-h)) < 0; j -= h) { \
                    T *dst = VEC_REF(M) *A##_static_get(vec, vec->first + j); \
                    T *src = VEC_REF(M) *A##_static_get(vec, vec->first + j-h); \
                    *dst = *src; \
                    /*a[j] = a[j - h]; */\
                } \
                /*a[j] = t; */\
                T *item = VEC_REF(M) *A##_static_get(vec, vec->first + j); \
                *item = temp; \
            } \
        } \
    }

/**
 * @brief A##_static_zero [COMMON] - set the vector struct without freeing to zero
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_STATIC_ZERO(N, A, T, F) \
    static inline void A##_static_zero(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        vec_memset(vec, 0, sizeof(*vec)); \
    }

/**
 * @brief A##_clear [COMMON] - set the vector length to zero without leaving it's allocated memory unchanged (aka. recycle)
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_CLEAR(N, A, T, F) \
    inline void A##_clear(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        vec->first = 0; \
        vec->last = 0; \
    }

/**
 * @brief A##_length [COMMON] - get the indexed vector length (in items)
 * @param vec - the vector
 * @return length in items
 */
#define VEC_IMPLEMENT_COMMON_LENGTH(N, A, T, F) \
    inline size_t A##_length(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        return vec->last - vec->first; \
    }

/**
 * @brief A##_capacity [COMMON] - get number of allocated item spaces (not in bytes)
 * @param vec - the vector
 * @return capacity in item spaces
 */
#define VEC_IMPLEMENT_COMMON_CAPACITY(N, A, T, F) \
    inline size_t A##_capacity(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        return vec->cap; \
    }

/**
 * @brief A##_empty [COMMON] - check if vector is empty
 * @param vec - the vector
 * @return boolean comparison: true if empty, false if not empty
 */
#define VEC_IMPLEMENT_COMMON_EMPTY(N, A, T, F) \
    inline int A##_empty(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        return (vec->first == vec->last); \
    }

/**
 * @brief A##_resize [COMMON] - resize vector to be able to hold a certain number of items
 * @param vec - the vector
 * @param cap - certain number of items
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_COMMON_RESIZE(N, A, T, F) \
    inline int A##_resize(N *vec, size_t cap) \
    { \
        VEC_ASSERT_REAL(vec); \
        int result = VEC_ERROR_NONE; \
        A##_static_shrink_front(vec); \
        result |= result ?: A##_reserve(vec, cap); \
        vec->last = cap; \
        result |= result ?: A##_static_shrink_back(vec); \
        return result; \
    }

/**
 * @brief A##_shrink [COMMON] - shrink memory usage to minimum number of items while still capable of holding the current number of items in use
 * @param vec - the vector
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_COMMON_SHRINK(N, A, T, F) \
    inline int A##_shrink(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        int result = A##_static_shrink_front(vec); \
        result |= A##_static_shrink_back(vec); \
        return result; \
    }

/**
 * @brief A##_iter_begin [COMMON] - get pointer to first item
 * @param vec - the vector
 * @return pointer to first item
 */
#define VEC_IMPLEMENT_COMMON_ITER_BEGIN(N, A, T, F, M) \
    inline VEC_ITEM(T, M)*A##_iter_begin(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(vec->first <= vec->last); \
        return vec->VEC_STRUCT_ITEMS + vec->first; \
    }

/**
 * @brief A##_iter_end [COMMON] - get pointer to one past last item
 * @param vec - the vector
 * @return pointer to one past last item
 */
#define VEC_IMPLEMENT_COMMON_ITER_END(N, A, T, F, M) \
    inline VEC_ITEM(T, M)*A##_iter_end(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(vec->first <= vec->last); \
        return vec->VEC_STRUCT_ITEMS + vec->last; \
    }

/**
 * @brief A##_iter_at [COMMON] - get pointer to certain item
 * @param vec - the vector
 * @return pointer to one past last item
 */
#define VEC_IMPLEMENT_COMMON_ITER_AT(N, A, T, F, M) \
    inline VEC_ITEM(T, M)*A##_iter_at(const N *vec, size_t index) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(vec->first + index <= vec->last); \
        return vec->VEC_STRUCT_ITEMS + vec->first + index; \
    }

/**
 * @brief A##_pop_front [COMMON] - pop one item from the front (and adjust first index)
 * @param vec - the vector
 * @return val - write back for popped value, pass 0 to ignore
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_POP_FRONT(N, A, T, F, M) \
    inline void A##_pop_front(N *vec, T *val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(vec->last > vec->first); \
        if(val) { \
            size_t first = vec->first; \
            T *item = VEC_REF(M) *A##_static_get(vec, first); \
            vec_memcpy(val, item, sizeof(T)); \
        } \
        vec->first++; \
    }

/**
 * @brief A##_pop_back [COMMON] - pop one item from the back (and adjust length)
 * @param vec - the vector
 * @param val - write back for popped value, pass 0 to ignore
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_POP_BACK(N, A, T, F, M) \
    inline void A##_pop_back(N *vec, T *val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(vec->last > vec->first); \
        if(val) { \
            size_t back = vec->last - 1; \
            T *item = VEC_REF(M) *A##_static_get(vec, back); \
            vec_memcpy(val, item, sizeof(T)); \
        } \
        vec->last--; \
    }

/**
 * @brief A##_get_at [COMMON] - get item at certain index
 * @param vec - the vector
 * @param index - the index
 * @return item (by value) at index
 */
#define VEC_IMPLEMENT_COMMON_GET_AT(N, A, T, F, M) \
    inline VEC_ITEM(T, M) A##_get_at(const N *vec, size_t index) \
    { \
        VEC_ASSERT_REAL(vec); \
        return *A##_static_get(vec, index + vec->first); \
    }

/**
 * @brief A##_get_front [COMMON] - get item at front
 * @param vec - the vector
 * @return item (by value) at front
 */
#define VEC_IMPLEMENT_COMMON_GET_FRONT(N, A, T, F, M) \
    inline VEC_ITEM(T, M) A##_get_front(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        return *A##_static_get(vec, vec->first); \
    }

/**
 * @brief A##_get_back [COMMON] - get item at end
 * @param vec - the vector
 * @return item (by value) at end
 */
#define VEC_IMPLEMENT_COMMON_GET_BACK(N, A, T, F, M) \
    inline VEC_ITEM(T, M) A##_get_back(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        return *A##_static_get(vec, vec->last - 1); \
    }

/**
 * @brief A##_set_at [COMMON] - overwrite one item at certain index
 * @param vec - the vector
 * @param index - the certain index
 * @param val - the value (by reference) to overwrite said index with
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_SET_AT(N, A, T, F, M) \
    inline void A##_set_at(N *vec, size_t index, VEC_ITEM(T, M) val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT(val, M); \
        T *item = VEC_REF(M) *A##_static_get(vec, index + vec->first); \
        if(F != 0) VEC_TYPE_FREE(F, item, T); \
        vec_memcpy(item, VEC_REF(M) val, sizeof(T)); \
    }

/**
 * @brief A##_pop_at [COMMON] - pop one item from the back (and adjust length)
 * @param vec - the vector
 * @param index - the index
 * @param val - write back for popped value, pass 0 to ignore
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_POP_AT(N, A, T, F, M) \
    inline void A##_pop_at(N *vec, size_t index, T *val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT(val, M); \
        VEC_ITEM(T, M) *item = A##_static_get(vec, index + vec->first); \
        if(val) { \
            vec_memcpy(val, VEC_REF(M) *item, sizeof(T)); \
        } \
        vec->last--; \
        vec_memmove(item, item + 1, sizeof(*item) * (vec->last - index - vec->first)); \
        return; \
    }

/**
 * @brief A##_push_at [COMMON] - add one item at index and move everything back
 * @param vec - the vector
 * @param index - the index
 * @param val - the value (by reference) to be written to said index
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_COMMON_PUSH_AT(N, A, T, F, M) \
    inline int A##_push_at(N *vec, size_t index, VEC_ITEM(T, M) val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT(val, M); \
        int result = A##_reserve(vec, vec->last + 1); \
        if(result) return result; \
        vec->last++; \
        VEC_ITEM(T, M) *item = A##_static_get(vec, index + vec->first); \
        vec_memmove(item + 1, item, sizeof(*item) * (vec->last - index - vec->first - 1)); \
        vec_memcpy(VEC_REF(M) *item, VEC_REF(M) val, sizeof(T)); \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_push_front [COMMON] - push one item from the front and move everything back
 * @param vec - the vector
 * @param val - the value (by reference) to be pushed
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_COMMON_PUSH_FRONT(N, A, T, F, M) \
    inline int A##_push_front(N *vec, VEC_ITEM(T, M) val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT(val, M); \
        int result = A##_reserve(vec, vec->last + 1); \
        if(result) return result; \
        size_t len = vec->last++ - vec->first; \
        VEC_ITEM(T, M) *item = A##_static_get(vec, vec->first); \
        vec_memmove(item + 1, item, sizeof(*item) * len); \
        vec_memcpy(VEC_REF(M) *item, VEC_REF(M) val, sizeof(T)); \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_push_back [COMMON] - push one item do the back
 * @param vec - the vector
 * @param val - the value (by reference) to be pushed
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_COMMON_PUSH_BACK(N, A, T, F, M) \
    inline int A##_push_back(N *vec, VEC_ITEM(T, M) val) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT(val, M); \
        int result = A##_reserve(vec, vec->last + 1); \
        if(result) return result; \
        size_t back = vec->last++; \
        T *item = VEC_REF(M) *A##_static_get(vec, back); \
        /* TODO add a new parameter: clearing function! so we don't free this! => we'll still overwrite 2 linew below... */ \
        if(F != 0) VEC_TYPE_FREE(F, item, T); /* required (?) in case we pop back (but don't free (?)) and then push back again */ \
        vec_memcpy(item, VEC_REF(M) val, sizeof(T)); \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_swap [COMMON] - swap two items at index 1 and 2
 * @param vec - the vector
 * @param i1 - index 1
 * @param i2 - index 2
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_SWAP(N, A, T, F, M) \
    inline void A##_swap(N *vec, size_t i1, size_t i2) \
    { \
        VEC_ASSERT_REAL(vec); \
        VEC_ITEM(T, M) *v1 = A##_static_get(vec, i1 + vec->first); \
        VEC_ITEM(T, M) *v2 = A##_static_get(vec, i2 + vec->first); \
        VEC_ITEM(T, M) tmp; \
        vec_memcpy(&tmp, v1, sizeof(tmp)); \
        vec_memcpy(v1, v2, sizeof(tmp)); \
        vec_memcpy(v2, &tmp, sizeof(tmp)); \
    }

/**
 * @brief A##_reverse [COMMON] - reverse a vector
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_COMMON_REVERSE(N, A, T, F) \
    inline void A##_reverse(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t len = A##_length(vec); \
        size_t len2 = len / 2; \
        for(size_t i = 0; i < len2; i++) { \
            A##_swap(vec, i, len - i - 1); \
        } \
    }

/* implementation by value */

/**
 * @brief A##_free [BY_VAL] - free the vector and all it's items
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_BY_VAL_FREE(N, A, T, F) \
    inline void A##_free(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        if(!vec->VEC_STRUCT_ITEMS) return; \
        /*printff(ERR_STRINGIFY(A) " freeing : %p", vec->VEC_STRUCT_ITEMS);*/\
        if(F != 0) { \
            for(size_t i = 0; i < vec->cap; i++) { \
                /*printff("  freeing : %p", &vec->VEC_STRUCT_ITEMS[i]);*/\
                VEC_TYPE_FREE(F, &vec->VEC_STRUCT_ITEMS[i], T); \
            } \
        } \
        free(vec->VEC_STRUCT_ITEMS); \
        A##_static_zero(vec); \
    }

/**
 * @brief A##_zero [BY_VAL] - set the vector struct without freeing to zero
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_BY_VAL_ZERO(N, A, T, F) \
    inline void A##_zero(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        vec_memset(vec, 0, sizeof(*vec)); \
    }

/**
 * @brief A##_reserved [BY_VAL] - get reserved number of bytes
 * @param vec - the vector struct
 * @return allocated size in bytes
 */
#define VEC_IMPLEMENT_BY_VAL_RESERVED(N, A, T, F) \
    inline size_t A##_reserved(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t result = 0; \
        result += (sizeof(*vec->VEC_STRUCT_ITEMS) * vec->cap); \
        return result; \
    }

/**
 * @brief A##_reserve [BY_VAL] - reserve memory for a certain minimum number of items (only expanding), doesn't change the indexed vector length
 * @param vec - the vector
 * @param cap - the minimum desired item capacity
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_VAL_RESERVE(N, A, T, F) \
    inline int A##_reserve(N *vec, size_t cap) \
    { \
        VEC_ASSERT_REAL(vec); \
        cap += VEC_KEEP_ZERO_END; \
        size_t cap_is = vec->cap; \
        if(cap > cap_is) { \
            size_t required = vec->cap ? vec->cap : VEC_DEFAULT_SIZE;\
            while(required < cap) required *= 2; \
            if(required > vec->cap) { \
                void *temp = vec_realloc(vec->VEC_STRUCT_ITEMS, sizeof(*vec->VEC_STRUCT_ITEMS) * required); \
                if(!temp) return VEC_ERROR_REALLOC; \
                vec->VEC_STRUCT_ITEMS = temp; \
                vec_memset(&vec->VEC_STRUCT_ITEMS[cap_is], 0, sizeof(*vec->VEC_STRUCT_ITEMS) * (required - cap_is)); \
                vec->cap = required; \
            } \
        } \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_copy [BY_VAL] - copy a vector and its items
 * @param dst - the destination vector
 * @param src - the source vector
 * @return zero if succes, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_VAL_COPY(N, A, T, F) \
    inline int A##_copy(N *dst, const N *src) \
    { \
        VEC_ASSERT_REAL(dst); \
        VEC_ASSERT_REAL(src); \
        VEC_ASSERT_REAL(dst != src); \
        A##_clear(dst); \
        if(A##_length(src)) { \
            int result = A##_reserve(dst, A##_length(src) + VEC_KEEP_ZERO_END); \
            if(result) return result; \
            vec_memcpy(A##_iter_begin(dst), A##_iter_begin(src), sizeof(*dst->VEC_STRUCT_ITEMS) * A##_length(src)); \
            dst->last = A##_length(src); \
        } \
        if(VEC_KEEP_ZERO_END) { \
            vec_memset(&dst->VEC_STRUCT_ITEMS[dst->last], 0, sizeof(*dst->VEC_STRUCT_ITEMS) * VEC_KEEP_ZERO_END); \
        } \
        return VEC_ERROR_NONE; \
    }


/* implementaiton by reference */

/**
 * @brief A##_free [BY_REF] - free the vector and all it's items
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_BY_REF_FREE(N, A, T, F) \
    inline void A##_free(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        for(size_t i = 0; i < vec->cap; i++) { \
            if(F != 0) { \
                VEC_TYPE_FREE(F, vec->VEC_STRUCT_ITEMS[i], T); \
            } \
            free(vec->VEC_STRUCT_ITEMS[i]); \
        } \
        free(vec->VEC_STRUCT_ITEMS); \
        A##_static_zero(vec); \
    }

/**
 * @brief A##_zero [BY_REF] - set the vector struct items without freeing to zero
 * @param vec - the vector
 * @return void
 */
#define VEC_IMPLEMENT_BY_REF_ZERO(N, A, T, F) \
    inline void A##_zero(N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        vec_memset(vec, 0, sizeof(*vec)); \
        /*for(size_t i = 0; i < vec->cap; i++) { \
            vec_memset(&vec->VEC_STRUCT_ITEMS[i], 0, sizeof(*vec->VEC_STRUCT_ITEMS)); \
        }*/ \
    }

/**
 * @brief A##_reserved [BY_REF] - get reserved number of bytes
 * @param vec - the vector struct
 * @return allocated size in bytes
 */
#define VEC_IMPLEMENT_BY_REF_RESERVED(N, A, T, F) \
    inline size_t A##_reserved(const N *vec) \
    { \
        VEC_ASSERT_REAL(vec); \
        size_t result = 0; \
        result += (sizeof(**vec->VEC_STRUCT_ITEMS) * vec->cap); \
        result += (sizeof(*vec->VEC_STRUCT_ITEMS) * vec->cap); \
        return result; \
    }

/**
 * @brief A##_reserve [BY_REF] - reserve memory for a certain minimum number of items (only expanding), doesn't change the indexed vector length
 * @param vec - the vector
 * @param cap - the minimum desired item capacity
 * @return zero if success, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_REF_RESERVE(N, A, T, F) \
    inline int A##_reserve(N *vec, size_t cap) \
    { \
        VEC_ASSERT_REAL(vec); \
        cap += VEC_KEEP_ZERO_END; \
        size_t cap_is = vec->cap; \
        size_t required = vec->cap ? vec->cap : VEC_DEFAULT_SIZE;\
        while(required < cap) required *= 2; \
        if(required > vec->cap) { \
            void *temp = vec_realloc(vec->VEC_STRUCT_ITEMS, sizeof(*vec->VEC_STRUCT_ITEMS) * required); \
            if(!temp) return VEC_ERROR_REALLOC; \
            vec->VEC_STRUCT_ITEMS = temp; \
            vec_memset(&vec->VEC_STRUCT_ITEMS[cap_is], 0, sizeof(*vec->VEC_STRUCT_ITEMS) * (required - cap_is)); \
            for(size_t i = cap_is; i < required; i++) { \
                vec->VEC_STRUCT_ITEMS[i] = vec_malloc(sizeof(**vec->VEC_STRUCT_ITEMS)); \
                if(!vec->VEC_STRUCT_ITEMS[i]) return VEC_ERROR_MALLOC; \
                vec_memset(vec->VEC_STRUCT_ITEMS[i], 0, sizeof(**vec->VEC_STRUCT_ITEMS)); \
            } \
            vec->cap = required; \
        } \
        return VEC_ERROR_NONE; \
    }

/**
 * @brief A##_copy [BY_REF] - copy a vector and its items
 * @param dst - the destination vector
 * @param src - the source vector
 * @return zero if succes, non-zero if failure
 */
#define VEC_IMPLEMENT_BY_REF_COPY(N, A, T, F) \
    inline int A##_copy(N *dst, const N *src) \
    { \
        VEC_ASSERT_REAL(dst); \
        VEC_ASSERT_REAL(src); \
        VEC_ASSERT_REAL(dst != src); \
        A##_clear(dst); \
        int result = 0; \
        if(A##_length(src)) { \
            result = A##_reserve(dst, A##_length(src)); \
            if(result) return result; \
            for(size_t i = 0; i < A##_length(src); i++) { \
                result |= result ?: A##_push_back(dst, A##_get_at(src, i)); \
            } \
        } \
        if(VEC_KEEP_ZERO_END) { \
            vec_memset(&dst->VEC_STRUCT_ITEMS[dst->last], 0, sizeof(*dst->VEC_STRUCT_ITEMS) * VEC_KEEP_ZERO_END); \
        } \
        return result; \
    }

#define VEC_IMPLEMENT_COMMON_EXTEND_FRONT(N, A, T, F, M) \
    int A##_extend_front(N *vec, N *v2) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = A##_length(v2); \
        size_t require = vec->last + until; \
        int result = A##_reserve(vec, require); \
        if(result) return result; \
        vec->last += until; \
        VEC_ITEM(T, M) *item = A##_static_get(vec, vec->first); \
        vec_memmove(item + until, item, sizeof(*item) * (vec->last - until - vec->first)); \
        /* TODO dangerous! memmove will not zero out the original memory bytes !!! */ \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(v2, i); \
            A##_set_at(vec, i, value); \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_BY_REF_EXTEND_BACK(N, A, T, F, M) \
    int A##_extend_back(N *vec, N *v2) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = A##_length(v2); \
        size_t require = vec->last + until; \
        size_t from = require < A##_length(vec) ? A##_length(vec) - require : 0; \
        int result = A##_reserve(vec, require); \
        if(result) return result; \
        vec->last += until; \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(v2, i); \
            A##_set_at(vec, from + i, value); \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_BY_VAL_EXTEND_BACK(N, A, T, F, M) \
    int A##_extend_back(N *vec, N *v2) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = A##_length(v2); \
        size_t require = vec->last + until; \
        /*size_t from = require < A##_length(vec) ? A##_length(vec) - require : 0;*/ \
        int result = A##_reserve(vec, require); \
        if(result) return result; \
        vec_memcpy(&vec->VEC_STRUCT_ITEMS[vec->last], A##_iter_begin(v2), sizeof(*vec->VEC_STRUCT_ITEMS) * until); \
        vec->last += until; \
        /*for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(v2, i); \
            A##_set_at(vec, from + i, value); \
        }*/\
        return VEC_ERROR_NONE; \
    }


#define VEC_IMPLEMENT_COMMON_EXTEND_AT(N, A, T, F, M) \
    int A##_extend_at(N *vec, size_t index, N *v2) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = A##_length(v2); \
        size_t require = vec->last + until; \
        int result = A##_reserve(vec, require); \
        if(result) return result; \
        vec->last += until; \
        VEC_ITEM(T, M) *item = A##_static_get(vec, index + vec->first); \
        vec_memmove(item + until, item, sizeof(*item) * (vec->last - index - until - vec->first)); \
        /* TODO dangerous! memmove will not zero out the original memory bytes !!! */ \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(v2, i); \
            A##_set_at(vec, index + i, value); \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_CUT_FRONT(N, A, T, F, M) \
    int A##_cut_front(N *vec, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = n < A##_length(vec) ? n : A##_length(vec); \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) *value = A##_static_get(vec, vec->first + i); \
            int result = A##_push_back(v2, *value); \
            if(result) return result; \
        } \
        vec->first += until; \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_CUT_BACK(N, A, T, F, M) \
    int A##_cut_back(N *vec, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t from = n < A##_length(vec) ? A##_length(vec) - n : 0; \
        size_t until = n < A##_length(vec) ? n : A##_length(vec); \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) *value = A##_static_get(vec, vec->first + i + from); /* optimization of this could also be interesting */ \
            int result = A##_push_back(v2, *value); \
            if(result) return result; \
        } \
        vec->last -= until; \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_CUT_AT(N, A, T, F, M) \
    int A##_cut_at(N *vec, size_t from, size_t n, N *v2) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        VEC_ASSERT_REAL(0 && "todo: implement"); \
        (void) from; \
        (void) n; \
        return 0; \
    }

#if 0
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = n < A##_length(vec) ? n : A##_length(vec); \
        VEC_ITEM(T, M) *item = A##_static_get(vec, from + vec->first); \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(vec, from + i); \
            int result = A##_push_back(v2, value); \
            if(result) return result; \
        } \
        if(VEC_IS_BY_REF(M) || F != 0) /* TODO actually confirm how this is optimized in higher levels of optimization */ { \
            /* similar to shrink front, we have to preserve all items, since they're alloced */ \
            /* TODO however, we don't want to keep any item data! we NEED to 0 it out, because v2' free will otherwise cause a dual free, in combination with vec's free... */ \
            VEC_ITEM(T, M) *residuals = malloc(sizeof(*residuals) * until); \
            if(!residuals) return VEC_ERROR_MALLOC; \ vec_memcpy(residuals, item, sizeof(*residuals) * until); \
            vec_memmove(item, item + until, sizeof(*item) * (vec->last - from)); \
            vec_memcpy(item + vec->last - until, residuals, sizeof(*residuals) * until); \
        } else { \
            vec_memmove(item, item + until, sizeof(*item) * (vec->last - from)); \
        } \
        vec->last -= until; \
        return VEC_ERROR_NONE; \
    }
#endif

#define VEC_IMPLEMENT_COMMON_PASTE_FRONT(N, A, T, F, M) \
    int A##_paste_front(N *vec, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = n < A##_length(v2) ? n : A##_length(v2); \
        size_t require = until < A##_length(vec) ? until : A##_length(vec); \
        int result = A##_reserve(vec, require); \
        if(result) return result; \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(v2, i); \
            A##_set_at(vec, i, value); \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_PASTE_BACK(N, A, T, F, M) \
    int A##_paste_back(N *vec, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = n < A##_length(v2) ? n : A##_length(v2); \
        size_t require = until < A##_length(vec) ? until : A##_length(vec); \
        size_t from = until < A##_length(vec) ? A##_length(vec) - until : 0; \
        int result = A##_reserve(vec, require); \
        if(result) return result; \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) value = A##_get_at(v2, i); \
            A##_set_at(vec, from + i, value); \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_PASTE_AT(N, A, T, F, M) \
    int A##_paste_at(N *vec, size_t index, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        VEC_ASSERT_REAL(0 && "todo: implement"); \
        (void) n; \
        (void) index; \
        return 0; \
    }

#define VEC_IMPLEMENT_COMMON_CAT_FRONT(N, A, T, F, M) \
    int A##_cat_front(N *vec, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = n < A##_length(vec) ? n : A##_length(vec); \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) *item = A##_static_get(vec, vec->first + i); \
            int result = A##_push_back(v2, *item); \
            if(result) return result; \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_CAT_BACK(N, A, T, F, M) \
    int A##_cat_back(N *vec, N *v2, size_t n) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t from = n < A##_length(vec) ? A##_length(vec) - n : 0; \
        size_t until = n < A##_length(vec) ? n : A##_length(vec); \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) *item = A##_static_get(vec, vec->first + i + from); \
            int result = A##_push_back(v2, *item); \
            if(result) return result; \
        } \
        return VEC_ERROR_NONE; \
    }

#define VEC_IMPLEMENT_COMMON_CAT_AT(N, A, T, F, M) \
    int A##_cat_at(N *vec, size_t from, size_t n, N *v2) { \
        VEC_ASSERT_REAL(vec); \
        VEC_ASSERT_REAL(v2); \
        size_t until = n < A##_length(vec) ? n : A##_length(vec); \
        for(size_t i = 0; i < until; i++) { \
            VEC_ITEM(T, M) *item = A##_static_get(vec, vec->first + i + from); \
            int result = A##_push_back(v2, *item); \
            if(result) return result; \
        } \
        return VEC_ERROR_NONE; \
    }


//#define VEC_H
//#endif

