#ifndef LUTS_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define ERR_LUTS_ADD    "failed adding value to lookup table"

#define LUTS_ITEM_BY_VAL(T)             T
#define LUTS_ITEM_BY_REF(T)             T *
#define LUTS_ITEM(T, M)                 LUTS_ITEM_##M(T)

#define LUTS_EMPTY                      SIZE_MAX
#define LUTS_WIDTH_MIN                  3

#define LUTS_SHIFT(width)               (width)
#define LUTS_CAP(width)                 (!!width * (size_t)1ULL << LUTS_SHIFT(width))


#define LUTS_TYPE_FREE(F,X,T,M)         ((void (*)(LUTS_ITEM(T,M)))(F))(X)

#define LUTS_TYPE_CMP(C,A,B,T,M)        ((int (*)(const LUTS_ITEM(T,M), const LUTS_ITEM(T,M)))(C))(A, B)

#define LUTS_PTR_BY_VAL   
#define LUTS_PTR_BY_REF                 &
#define LUTS_PTR(M)                     LUTS_PTR_##M

#define LUTS_REF_BY_VAL                 &
#define LUTS_REF_BY_REF
#define LUTS_REF(M)                     LUTS_REF_##M

#define LUTS_IS_BY_REF_BY_REF           1
#define LUTS_IS_BY_REF_BY_VAL           0
#define LUTS_IS_BY_REF(M)               LUTS_IS_BY_REF_##M

#define LUTS_ASSERT_ARG_M_BY_VAL(v)     do {} while(0)
#define LUTS_ASSERT_ARG_M_BY_REF(v)     LUTS_ASSERT_REAL(v, "expecting argument")
#define LUTS_ASSERT_ARG_M(v, M)         LUTS_ASSERT_ARG_M_##M(v)
#define LUTS_ASSERT_ARG(v)              LUTS_ASSERT_REAL(v, "expecting argument")
#define LUTS_ASSERT_REAL(v, msg)        assert(v && msg)

#define LUTS_INCLUDE(N, A, TK, MK, TV, MV) \
    typedef struct N##Item { \
        LUTS_ITEM(TK, MK) key; \
        LUTS_ITEM(TV, MV) val; \
        size_t hash; \
    } N##Item; \
    typedef struct N { \
        N##Item **buckets; \
        size_t used; \
        uint8_t width; \
    } N; \
    \
    void A##_free(N *lut); \
    int A##_grow(N *lut, size_t width); \
    int A##_set(N *lut, const LUTS_ITEM(TK, MK) key, LUTS_ITEM(TV, MV) val); \
    TV *A##_get(N *lut, const LUTS_ITEM(TK, MK) key); \
    void A##_del(N *lut, const LUTS_ITEM(TK, MK) key); \
    void A##_clear(N *lut); \

#define LUTS_IMPLEMENT(N, A, TK, MK, TV, MV, H, C, FK, FV)   \
    LUTS_IMPLEMENT_COMMON_STATIC_GET_ITEM(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    LUTS_IMPLEMENT_COMMON_FREE(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    LUTS_IMPLEMENT_COMMON_GROW(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    LUTS_IMPLEMENT_COMMON_SET(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    LUTS_IMPLEMENT_COMMON_GET(N, A, TK, MK, TV, MV, H, C, FK, FV) \

#define LUTS_IMPLEMENT_COMMON_STATIC_GET_ITEM(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    static N##Item **A##_static_get_item(N *lut, const LUTS_ITEM(TK, MK) key, size_t hash, bool intend_to_set) { \
        LUTS_ASSERT_ARG(lut); \
        LUTS_ASSERT_ARG_M(key, MK); \
        size_t perturb = hash >> 5; \
        size_t mask = ~(SIZE_MAX << LUTS_SHIFT(lut->width)); \
        size_t i = mask & hash; \
        N##Item **item = &lut->buckets[i]; \
        for(;;) { \
            /*printff("  %zu", i);*/\
            if(!*item) break; \
            if(intend_to_set && (*item)->hash == LUTS_EMPTY) break; \
            if((*item)->hash == hash) { \
                if(C != 0) { if(!LUTS_TYPE_CMP(C, (*item)->key, key, TK, MK)) return item; } \
                else { if(!memcmp(item, LUTS_REF(MK)key, sizeof(*LUTS_REF(MK)key))) return item; } \
            } \
            perturb >>= 5; \
            i = mask & (i * 5 + perturb + 1); \
            /* get NEXT item */ \
            item = &lut->buckets[i]; \
        } \
        return item; \
    }

#define LUTS_IMPLEMENT_COMMON_FREE(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    void A##_free(N *lut) { \
        ASSERT_ARG(lut); \
        for(size_t i = 0; i < LUTS_CAP(lut->width); ++i) { \
            N##Item **item = &lut->buckets[i]; \
            if(*item) { \
                if(FK != 0) LUTS_TYPE_FREE(FK, (*item)->key, TK, MK); \
                if(FV != 0) LUTS_TYPE_FREE(FV, (*item)->val, TV, MV); \
            } \
            free(*item); \
        } \
        free(lut->buckets); \
        memset(lut, 0, sizeof(*lut)); \
    }

#define LUTS_IMPLEMENT_COMMON_GROW(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    int A##_grow(N *lut, size_t width) { \
        LUTS_ASSERT_ARG(lut); \
        LUTS_ASSERT_REAL(width > lut->width, "expect larger new width then current"); \
        if(width <= lut->width) return -1; \
        if(width < LUTS_WIDTH_MIN) width = LUTS_WIDTH_MIN; \
        /*printff("NEW WIDTH %zu", width);*/ \
        N grown = {0}; \
        grown.buckets = malloc(sizeof(grown.buckets) * LUTS_CAP(width)); \
        if(!grown.buckets) return -1; \
        grown.width = width; \
        grown.used = lut->used; \
        memset(grown.buckets, 0, sizeof(grown.buckets) * LUTS_CAP(width)); \
        /* re-add values */ \
        for(size_t i = 0; i < LUTS_CAP(lut->width); ++i) { \
            N##Item *src = lut->buckets[i]; \
            if(!src) continue; \
            if(src->hash == LUTS_EMPTY) { \
                if(src) { \
                    /*str_free(src->val); \
                    str_free(src->key); \
                    TODO: do this in del */ \
                    free(src); \
                } \
                continue; \
            } \
            size_t hash = src->hash; \
            N##Item **item = A##_static_get_item(&grown, src->key, hash, true); \
            *item = src; \
        } \
        free(lut->buckets); \
        /* assign grown table */ \
        *lut = grown; \
        return 0; \
    }

#define LUTS_IMPLEMENT_COMMON_SET(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    int A##_set(N *lut, const LUTS_ITEM(TK, MK) key, LUTS_ITEM(TV, MV) val) { \
        LUTS_ASSERT_ARG(lut); \
        LUTS_ASSERT_ARG_M(key, MK); \
        if(2 * lut->used >= LUTS_CAP(lut->width)) { \
            if(A##_grow(lut, lut->width + 3)) return -1; \
            /*if(A##_grow(lut, 20)) return -1;*/ \
        } \
        size_t hash = H(key); \
        N##Item **item = A##_static_get_item(lut, key, hash, true); \
        if(*item) { \
            /* FREE OLD KEY */ \
        } else { \
            size_t req = sizeof(**item); \
            if(LUTS_IS_BY_REF(MK)) { \
                req += sizeof(*LUTS_REF(MK)(*item)->key); \
            } \
            if(LUTS_IS_BY_REF(MV)) { \
                req += sizeof(*LUTS_REF(MK)(*item)->val); \
            } \
            *item = malloc(req); \
            memset(*item, 0, sizeof(**item)); \
            if(!*item) return -1; \
            if(LUTS_IS_BY_REF(MK)) { \
                void *p = (void *)*item + sizeof(**item) + 0; \
                memset(p, 0, sizeof((*item)->key)); \
                memcpy(&(*item)->key, &p, sizeof((*item)->key)); \
            } \
            if(LUTS_IS_BY_REF(MV)) { \
                void *p = (void *)*item + sizeof(**item) + sizeof(*LUTS_REF(MK)(*item)->key); \
                memset(p, 0, sizeof((*item)->val)); \
                memcpy(&(*item)->val, &p, sizeof((*item)->val)); \
            } \
        } \
        memcpy(LUTS_REF(MK)(*item)->key, LUTS_REF(MK)key, sizeof(TK)); \
        memcpy(LUTS_REF(MV)(*item)->val, LUTS_REF(MV)val, sizeof(TV)); \
        (*item)->hash = hash; \
        ++lut->used; \
        return 0; \
    }

#define LUTS_IMPLEMENT_COMMON_GET(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    TV *A##_get(N *lut, const LUTS_ITEM(TK, MK) key) { \
        LUTS_ASSERT_ARG(lut); \
        LUTS_ASSERT_ARG_M(key, MK); \
        size_t hash = H(key); \
        N##Item *item = *A##_static_get_item(lut, key, hash, false); \
        return item ? LUTS_REF(MV)item->val : 0; \
    }

#define LUTS_IMPLEMENT_COMMON_DEL(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    void A##_del(N *lut, const LUTS_ITEM(TK, MK) key) { \
        LUTS_ASSERT_ARG(lut); \
        LUTS_ASSERT_ARG_M(key, TK); \
        size_t hash = H(key); \
        N##Item *item = *A##_static_get_item(lut, key, hash, true); \
        if(item) { \
            item->hash = LUTS_EMPTY; \
            if(FK != 0) LUTS_TYPE_FREE(FK, LUTS_PTR(MK)item->key, TK, MK); \
            if(FV != 0) LUTS_TYPE_FREE(FV, LUTS_PTR(MV)item->val, TV, MV); \
        } \
    }

#define LUTS_IMPLEMENT_COMMON_CLEAR(N, A, TK, MK, TV, MV, H, C, FK, FV) \
    void A##_clear(N *lut) { \
        LUTS_ASSERT_ARG(lut); \
        for(size_t i = 0; i < LUTS_CAP(lut->width); ++i) { \
            N##Item *item = lut->buckets[i]; \
            if(!item) continue; \
            item->hash = LUT_EMPTY; \
        } \
    }


#define LUTS_H
#endif

