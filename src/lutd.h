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

#ifndef LUTD_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define STATIC_ASSERT(...) _Static_assert(__VA_ARGS__)

typedef enum {
    LUTD_ERROR_NONE,
    /* errors below */
    LUTD_ERROR_MALLOC = -1,
    LUTD_ERROR_REALLOC = -2,
    LUTD_ERROR_EXPECT_NULL = -3,
    LUTD_ERROR_INIT = -4,
} LUTDErrorList;

#define ERR_LUTD_INIT "failed initializing lookup table"
#define ERR_LUTD_ADD "failed adding item to lookup table"
#define ERR_LUTD_FIND "failed finding node in lookup table"
#define ERR_LUTD_DUMP "failed dumping lookup table"

#define LUTD_DEFAULT_SIZE    4

#define LUTD_CAST_FREE(X)        ((void *)(X))
#define LUTD_TYPE_FREE(F,X,T)    ((void (*)(T *))(F))(LUTD_CAST_FREE(X))

#define LUTD_TYPE_CMP(C,A,B,T,M)   ((int (*)(const LUTD_ITEM(T,M), const LUTD_ITEM(T,M)))(C))(A, B)

/* N = name
 * A = abbreviation
 * T = type
 * W = width - 2 ^ W = bits
 * M = mode - either BY_VAL or BY_REF
 * H = hash function
 * C = cmp function
 * F = free function (optional)
 * D = dump function (optional, required if T is a struct ???)
 */

#define LUTD_ITEM_BY_VAL(T) T
#define LUTD_ITEM_BY_REF(T) T *
#define LUTD_ITEM(T, M)     LUTD_ITEM_##M(T)

#define LUTD_ASSERT_REAL(x)     assert(x && "assertion failed")

#define LUTD_ASSERT_BY_VAL(v)   do {} while(0)
#define LUTD_ASSERT_BY_REF(v)   LUTD_ASSERT_REAL(v)
#define LUTD_ASSERT(M, v)       LUTD_ASSERT_##M(v)
#define LUTD_ASSERT_INIT(T)     LUTD_ASSERT_REAL((T)->buckets && "not initialized")

#define LUTD_REF_BY_VAL   &
#define LUTD_REF_BY_REF
#define LUTD_REF(M)       LUTD_REF_##M

#define LUTD_IS_BY_REF_BY_REF 1
#define LUTD_IS_BY_REF_BY_VAL 0
#define LUTD_IS_BY_REF(M)     LUTD_IS_BY_REF_##M

#define LUTD_INCLUDE(N, A, T, M) \
    typedef struct N##Bucket { \
        size_t cap; \
        size_t len; \
        size_t *count; \
        LUTD_ITEM(T, M)*items; \
    } N##Bucket; \
    typedef struct { \
        N##Bucket *buckets; \
        size_t width; \
    } N; /* if free() takes too long: add an array that holds each item index that actually needs to be freed */ \
    \
    int A##_init(N *l, size_t width); \
    int A##_join(N *l, N *arr, size_t n); \
    int A##_add(N *l, LUTD_ITEM(T, M) v); \
    int A##_add_count(N *l, LUTD_ITEM(T, M) v, size_t count); \
    int A##_del(N *l, LUTD_ITEM(T, M) v); \
    bool A##_has(N *l, LUTD_ITEM(T, M) v); \
    int A##_find(N *l, LUTD_ITEM(T, M) v, size_t *i, size_t *j); \
    void A##_free(N *l); \
    int A##_clear(N *l); \
    int A##_dump(N *l, LUTD_ITEM(T, M) **arr, size_t **counts, size_t *len); \
    bool A##_empty(N *l); \
    size_t A##_length(N *l);

#define LUTD_IMPLEMENT(N, A, T, M, H, C, F) \
    LUTD_IMPLEMENT_##M(N, A, T, H, C, F); \
    /*LUTD_IMPLEMENT_COMMON_STATIC_THREAD_JOIN(N, A, T, C, F);*/ \
    /*LUTD_IMPLEMENT_COMMON_JOIN(N, A, T, C, F);*/ \
    LUTD_IMPLEMENT_COMMON_INIT(N, A, T, C, F) \
    LUTD_IMPLEMENT_COMMON_CLEAR(N, A, T, C, F) \
    LUTD_IMPLEMENT_COMMON_ADD(N, A, T, M, H, C, F) \
    LUTD_IMPLEMENT_COMMON_ADD_COUNT(N, A, T, M, H, C, F) \
    LUTD_IMPLEMENT_COMMON_HAS(N, A, T, M, H, C, F) \
    LUTD_IMPLEMENT_COMMON_FIND(N, A, T, M, H, C, F) \
    LUTD_IMPLEMENT_COMMON_DEL(N, A, T, M, H, C, F) \
    LUTD_IMPLEMENT_COMMON_DUMP(N, A, T, M, C, F) \
    LUTD_IMPLEMENT_COMMON_EMPTY(N, A, T, M, C, F) \
    LUTD_IMPLEMENT_COMMON_LENGTH(N, A, T, M, C, F) \

#define LUTD_IMPLEMENT_BY_VAL(N, A, T, H, C, F) \
    LUTD_IMPLEMENT_BY_VAL_RESERVE(N, A, T, H, C, F) \
    LUTD_IMPLEMENT_BY_VAL_FREE(N, A, T, C, F) \

#define LUTD_IMPLEMENT_BY_REF(N, A, T, H, C, F) \
    LUTD_IMPLEMENT_BY_REF_RESERVE(N, A, T, H, C, F) \
    LUTD_IMPLEMENT_BY_REF_FREE(N, A, T, C, F) \

/* implementation for both */

/******************************************************************************/
/* PUBLIC FUNCTION IMPLEMENTATIONS ********************************************/
/******************************************************************************/

/* implementation by value */

#define LUTD_IMPLEMENT_BY_VAL_RESERVE(N, A, T, H, C, F) \
    int A##_static_reserve(N *l, size_t hash, size_t exist_index, size_t cap) \
    { \
        size_t len = l->buckets[hash].cap; \
        size_t required = len ? len : LUTD_DEFAULT_SIZE;\
        while(required < cap) required *= 2; \
        if(required > len) { \
            /* buckets */ \
            void *temp = realloc(l->buckets[hash].items, sizeof(T) * required); \
            if(!temp) return LUTD_ERROR_REALLOC; \
            l->buckets[hash].items = temp; \
            memset(&l->buckets[hash].items[exist_index], 0, sizeof(T) * (required - len)); \
            /* counts */ \
            temp = realloc(l->buckets[hash].count, sizeof(size_t) * required); \
            if(!temp) return LUTD_ERROR_REALLOC; \
            l->buckets[hash].count = temp; \
            memset(&l->buckets[hash].count[exist_index], 0, sizeof(size_t) * (required - len)); \
            /* finish up */ \
            l->buckets[hash].cap = required; \
        } \
        return 0; \
    }

#define LUTD_IMPLEMENT_BY_VAL_FREE(N, A, T, C, F) \
    void A##_free(N *l) \
    { \
        LUTD_ASSERT_REAL(l); \
        if(!l->width) return; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            for(size_t j = 0; j < l->buckets[i].cap; j++) { \
                /* NOTE this is ugly, provide a way to give a clear function for sub items... */ \
                if(F != 0) LUTD_TYPE_FREE(F, &l->buckets[i].items[j], T); \
            } \
            free(l->buckets[i].items); \
            free(l->buckets[i].count); \
        } \
        memset(l->buckets, 0, sizeof(*l->buckets) * (1ULL << (l->width - 1))); \
        free(l->buckets); \
        memset(l, 0, sizeof(*l)); \
    }

/* implementation by reference */

#define LUTD_IMPLEMENT_BY_REF_RESERVE(N, A, T, H, C, F) \
    int A##_static_reserve(N *l, size_t hash, size_t exist_index, size_t cap) \
    { \
        size_t len = l->buckets[hash].cap; \
        size_t required = len ? len : LUTD_DEFAULT_SIZE;\
        while(required < cap) required *= 2; \
        if(required > len) { \
            /* buckets */ \
            void *temp = realloc(l->buckets[hash].items, sizeof(T) * required); \
            if(!temp) return LUTD_ERROR_REALLOC; \
            l->buckets[hash].items = temp; \
            memset(&l->buckets[hash].items[exist_index], 0, sizeof(T) * (required - len)); \
            for(size_t i = len; i < required; i++) { \
                l->buckets[hash].items[i] = malloc(sizeof(**l->buckets[hash].items)); \
                if(!l->buckets[hash].items[i]) return LUTD_ERROR_MALLOC; \
                memset(l->buckets[hash].items[i], 0, sizeof(**l->buckets[hash].items)); \
            } \
            /* counts */ \
            temp = realloc(l->buckets[hash].count, sizeof(size_t) * required); \
            if(!temp) return LUTD_ERROR_REALLOC; \
            l->buckets[hash].count = temp; \
            memset(&l->buckets[hash].count[exist_index], 0, sizeof(size_t) * (required - len)); \
            /* finish up */ \
            l->buckets[hash].cap = required; \
        } \
        return 0; \
    }

#define LUTD_IMPLEMENT_BY_REF_FREE(N, A, T, C, F) \
    void A##_free(N *l) \
    { \
        LUTD_ASSERT_REAL(l); \
        if(!l->width) return; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            for(size_t j = 0; j < l->buckets[i].cap; j++) { \
                /* NOTE this is ugly, provide a way to give a clear function for sub items... */ \
                if(F != 0) LUTD_TYPE_FREE(F, l->buckets[i].items[j], T); \
                free(l->buckets[i].items[j]); \
            } \
            free(l->buckets[i].items); \
            free(l->buckets[i].count); \
        } \
        memset(l->buckets, 0, sizeof(*l->buckets) * (1ULL << (l->width - 1))); \
        free(l->buckets); \
        memset(l, 0, sizeof(*l)); \
    }

/* implementation for both */

#define LUTD_IMPLEMENT_COMMON_INIT(N, A, T, C, F) \
    int A##_init(N *l, size_t width) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT_REAL(width < 8 * sizeof(size_t)); \
        /*A##_clear(l);*/ \
        void *temp = realloc(l->buckets, sizeof(*l->buckets) * (1ULL << (width - 1))); \
        if(!temp) return -1; \
        l->buckets = temp; \
        l->width = width; \
        for(size_t i = 0; i < 1ULL << (width - 1); i++) { \
            memset(&l->buckets[i], 0, sizeof(l->buckets[i])); \
        } \
        return 0; \
    }

#define LUTD_IMPLEMENT_COMMON_CLEAR(N, A, T, C, F) \
    int A##_clear(N *l) \
    { \
        LUTD_ASSERT_REAL(l); \
        if(!l->width) return 0; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            for(size_t j = 0; j < l->buckets[i].cap; j++) { \
                /* NOTE this is ugly, provide a way to give a clear function for sub items... */ \
                if(F != 0) LUTD_TYPE_FREE(F, &l->buckets[i].items[j], T); \
            } \
            l->buckets[i].len = 0; \
        } \
        return 0;   \
    }

#define LUTD_IMPLEMENT_COMMON_ADD(N, A, T, M, H, C, F) \
    int A##_add(N *l, LUTD_ITEM(T, M) v) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT(M, v); \
        int result = A##_add_count(l, v, 1); \
        return result; \
    }

#define LUTD_IMPLEMENT_COMMON_ADD_COUNT(N, A, T, M, H, C, F) \
    int A##_add_count(N *l, LUTD_ITEM(T, M) v, size_t count) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT(M, v); \
        LUTD_ASSERT_INIT(l); \
        if(!l->width) return LUTD_ERROR_INIT; \
        bool exists = false; \
        size_t hash = H(v) % (1ULL << (l->width - 1)); /* TODO this is stupid. */ \
        size_t exist_index = 0; \
        for(exist_index = 0; exist_index < l->buckets[hash].len; exist_index++) { \
            if(C != 0) { if(LUTD_TYPE_CMP(C, l->buckets[hash].items[exist_index], v, T, M)) continue; } \
            else { if(memcmp(&l->buckets[hash].items[exist_index], &v, sizeof(v))) continue; } \
            exists = true; \
            break; \
        } \
        if(!exists) { \
            size_t cap = exist_index + 1; \
            if(A##_static_reserve(l, hash, exist_index, cap)) return LUTD_ERROR_REALLOC; \
            T *item = LUTD_REF(M) l->buckets[hash].items[exist_index];\
            memcpy(item, LUTD_REF(M) v, sizeof(T)); \
            l->buckets[hash].count[exist_index] = 0; \
            l->buckets[hash].len++; \
        } \
        l->buckets[hash].count[exist_index] += count; \
        return 0; \
    }

#define LUTD_IMPLEMENT_COMMON_HAS(N, A, T, M, H, C, F) \
    bool A##_has(N *l, LUTD_ITEM(T, M) v) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT(M, v); \
        LUTD_ASSERT_INIT(l); \
        if(!l->width) return false; \
        bool exists = false; \
        size_t hash = H(v) % (1ULL << (l->width - 1)); /* TODO this is stupid. */ \
        size_t exist_index = 0; \
        for(exist_index = 0; exist_index < l->buckets[hash].len; exist_index++) { \
            if(C != 0) { if(LUTD_TYPE_CMP(C, l->buckets[hash].items[exist_index], v, T, M)) continue; } \
            else { if(memcmp(&l->buckets[hash].items[exist_index], &v, sizeof(v))) continue; } \
            exists = true; \
            break; \
        } \
        return exists; \
    }

#define LUTD_IMPLEMENT_COMMON_FIND(N, A, T, M, H, C, F) \
    int A##_find(N *l, LUTD_ITEM(T, M) v, size_t *i, size_t *j) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT_REAL(i); \
        LUTD_ASSERT_REAL(j); \
        LUTD_ASSERT(M, v); \
        LUTD_ASSERT_INIT(l); \
        if(!l->width) return LUTD_ERROR_INIT; \
        size_t hash = H(v) % (1ULL << (l->width - 1)); /* TODO this is stupid. */ \
        size_t exist_index = 0; \
        for(exist_index = 0; exist_index < l->buckets[hash].len; exist_index++) { \
            if(C != 0) { if(LUTD_TYPE_CMP(C, l->buckets[hash].items[exist_index], v, T, M)) continue; } \
            else { if(memcmp(&l->buckets[hash].items[exist_index], &v, sizeof(v))) continue; } \
            *i = hash; \
            *j = exist_index; \
            return 0; \
        } \
        return -1; \
    }

#if 0
#include <pthread.h>
#define N_THREADS   16 /* TODO make this variable */

#define LUTD_IMPLEMENT_COMMON_STATIC_THREAD_JOIN(N, A, T, F) \
    typedef struct { \
        N *src; \
        N *dst; \
        size_t i_bucket; \
        pthread_mutex_t *mutex; \
    } Thread##A##Join; \
    void *A##_static_thread_join(void *args) { \
        LUTD_ASSERT_REAL(args); \
        Thread##A##Join *tj = args; \
        int result = 0; \
        if(tj->i_bucket >= 1ULL << (tj->src->width - 1)) { \
            return 0; /* TODO return proper code!? */ \
        } \
        for(size_t j = 0; j < tj->src->buckets[tj->i_bucket].len; j++) { \
            T item = tj->src->buckets[tj->i_bucket].items[j]; \
            size_t count = tj->src->buckets[tj->i_bucket].count[j]; \
            pthread_mutex_lock(tj->mutex); \
            result |= result ?: A##_add_count(tj->dst, item, count); \
            pthread_mutex_unlock(tj->mutex); \
            if(result) break; \
        } \
        /* TODO fix this shit: return (void *)(uintptr_t)(size_t)result;*/ \
        return 0; \
    }

#define LUTD_IMPLEMENT_COMMON_JOIN(N, A, T, C, F) \
    int A##_join(N *l, N *arr, size_t n) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT_REAL(arr); \
        LUTD_ASSERT_REAL(l->width); \
        pthread_t thread_ids[N_THREADS] = {0}; \
        pthread_attr_t thread_attr; \
        pthread_attr_init(&thread_attr); \
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE); \
        pthread_mutex_t thread_mutex; \
        pthread_mutex_init(&thread_mutex, 0); \
        Thread##A##Join tj[N_THREADS] = {0}; \
        for(size_t k = 0; k < n; k++) { \
            LUTD_ASSERT_REAL(arr[k].width == l->width); \
            for(size_t i = 0; i < 1ULL << (l->width - 1); i += N_THREADS) { \
                for(size_t j = 0; j < N_THREADS; j++) { \
                    tj[j].src = &arr[k]; \
                    tj[j].dst = l; \
                    tj[j].i_bucket = i; \
                    tj[j].mutex = &thread_mutex; \
                    pthread_create(&thread_ids[j], 0, A##_static_thread_join, &tj[j]); \
                } \
                for(size_t j = 0; j < N_THREADS; j++) { \
                    pthread_join(thread_ids[j], 0); \
                } \
                /* MULTITHREAD THIS
                for(size_t j = 0; j < arr[k].buckets[i].len; j++) { \
                    T item = arr[k].buckets[i].items[j]; \
                    size_t count = arr[k].buckets[i].count[j]; \
                    result |= result ?: A##_add_count(l, item, count); \
                } */ \
            } \
        } \
        /* TODO fix this shit: return (void *)(uintptr_t)(size_t)result;*/ \
        pthread_mutex_destroy(&thread_mutex); \
        return 0; \
    }

#else
/* TODO provide the blow NO THREAD VERSION */
#define LUTD_IMPLEMENT_COMMON_JOIN(N, A, T, C, F) \
    int A##_join(N *l, N *arr, size_t n) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT_REAL(arr); \
        LUTD_ASSERT_REAL(l->width); \
        int result = 0; \
        for(size_t k = 0; k < n; k++) { \
            LUTD_ASSERT_REAL(arr[k].width == l->width); \
            for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
                for(size_t j = 0; j < arr[k].buckets[i].len; j++) { \
                    T item = arr[k].buckets[i].items[j]; \
                    size_t count = arr[k].buckets[i].count[j]; \
                    result |= result ?: A##_add_count(l, item, count); \
                } \
            } \
        } \
        return 0; \
    }
#endif

#define LUTD_IMPLEMENT_COMMON_DEL(N, A, T, M, H, C, F) \
    int A##_del(N *l, LUTD_ITEM(T, M) v) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT(M, v); \
        bool exists = false; \
        size_t hash = H(v) % (1ULL << (l->width - 1)); \
        size_t exist_index = 0; \
        for(exist_index = 0; exist_index < l->buckets[hash].len; exist_index++) { \
            if(C != 0) { if(LUTD_TYPE_CMP(C, l->buckets[hash].items[exist_index], v, T, M)) continue; } \
            else { if(memcmp(&l->buckets[hash].items[exist_index], &v, sizeof(v))) continue; } \
            exists = true; \
            break; \
        } \
        if(exists) { \
            size_t len = l->buckets[hash].len; \
            if(len - exist_index > 1) { \
                if(F != 0) { \
                    LUTD_TYPE_FREE(F, &l->buckets[hash].items[exist_index], T); \
                } \
                if(LUTD_IS_BY_REF(M)) { \
                    /* preserve current index so we don't have to free it */ \
                    T *temp = LUTD_REF(M) l->buckets[hash].items[exist_index]; \
                    memmove(&l->buckets[hash].items[exist_index], &l->buckets[hash].items[1 + exist_index], sizeof(*l->buckets->items) * (len - exist_index - 1)); \
                    memmove(&l->buckets[hash].count[exist_index], &l->buckets[hash].count[1 + exist_index], sizeof(size_t) * (len - exist_index - 1)); \
                    memcpy(&l->buckets[hash].items[len - 1], &temp, sizeof(*l->buckets->items)); \
                    /* TODO do I also have to zero this temp ?? */ \
                } else { \
                    memmove(&l->buckets[hash].items[exist_index], &l->buckets[hash].items[1 + exist_index], sizeof(*l->buckets->items) * (len - exist_index - 1)); \
                    memmove(&l->buckets[hash].count[exist_index], &l->buckets[hash].count[1 + exist_index], sizeof(size_t) * (len - exist_index - 1)); \
                } \
            } \
            l->buckets[hash].len--; \
        } \
        return 0;   \
    }

#define LUTD_IMPLEMENT_COMMON_DUMP(N, A, T, M, C, F) \
    int A##_dump(N *l, LUTD_ITEM(T, M) **arr, size_t **counts, size_t *len) \
    { \
        LUTD_ASSERT_REAL(l); \
        LUTD_ASSERT_REAL(arr); \
        LUTD_ASSERT_REAL(len); \
        LUTD_ASSERT_INIT(l); \
        if(!l->width) return LUTD_ERROR_INIT; \
        size_t used_total = 0; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            used_total += l->buckets[i].len; \
        } \
        if(*arr) return LUTD_ERROR_EXPECT_NULL; \
        if(counts) { \
            if(*counts) return LUTD_ERROR_EXPECT_NULL; \
        } \
        *arr = malloc(sizeof(T) * used_total); \
        if(!*arr) return LUTD_ERROR_MALLOC; \
        if(counts) { \
            *counts = malloc(sizeof(size_t) * used_total); \
            if(!*counts) return LUTD_ERROR_MALLOC; \
        } \
        *len = used_total; \
        size_t used_index = 0; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            for(size_t j = 0; j < l->buckets[i].len; j++) { \
                (*arr)[used_index] = l->buckets[i].items[j]; \
                if(counts) { \
                    (*counts)[used_index] = l->buckets[i].count[j]; \
                } \
                used_index++; \
            } \
        } \
        return 0;   \
    }

#define LUTD_IMPLEMENT_COMMON_EMPTY(N, A, T, M, C, F) \
    bool A##_empty(N *l) { \
        LUTD_ASSERT_REAL(l); \
        if(!l->width) return true; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            for(size_t j = 0; j < l->buckets[i].cap; j++) { \
                if(l->buckets[i].count[j]) return false; \
            } \
        } \
        return true; \
    }

#define LUTD_IMPLEMENT_COMMON_LENGTH(N, A, T, M, C, F) \
    size_t A##_length(N *l) { \
        LUTD_ASSERT_REAL(l); \
        if(!l->width) return 0; \
        size_t result = 0; \
        for(size_t i = 0; i < 1ULL << (l->width - 1); i++) { \
            for(size_t j = 0; j < l->buckets[i].cap; j++) { \
                result += (l->buckets[i].count[j]); \
            } \
        } \
        return result; \
    }


#define LUTD_H
#endif


