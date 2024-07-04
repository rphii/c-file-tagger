#ifndef ATTR_H

#include "platform.h"

/* https://stackoverflow.com/questions/28166565/detect-gcc-as-opposed-to-msvc-clang-with-macro */
#if defined(PLATFORM_LINUX) && defined(__GNUC__) && !defined(__clang__)
/* this preprocessor thing is mainly to disable clang lsp messages */
/* TODO add clang's way of handling the ones below? */

#define ATTR_FALLTHROUGH        [[fallthrough]]
#define ATTR_NODISCARD          [[nodiscard]]

#define ATTR_NORETURN           __attribute__((noreturn)) /* this might have a c23 attribute... */
#define ATTR_NOTHROW            __attribute__((nothrow))
#define ATTR_NONNULL /* TODO */

#define ATTR_PURE               __attribute__((pure)) /* output is solely based on their inputs; does dereference; e.g. hash, strlen, memcmp */
#define ATTR_CONST              __attribute__((const)) /* output is solely based on their inputs; doesn't dereference; e.g. abs, sqrt, exp */
#define ATTR_COLD               __attribute__((cold))
#define ATTR_HOT                __attribute__((hot))

#define ATTR_FORMAT(s,c)        __attribute__((format (printf, s, c))) /* s=string, c=1st to check */

/* just don't use those and you're good */
#define ATTR_EXPECT_TRUE(x)     __builtin_expect((x), 1)
#define ATTR_EXPECT_FALSE(x)    __builtin_expect((x), 0) 

#else
#define ATTR_FALLTHROUGH
#define ATTR_NODISCARD

#define ATTR_NORETURN
#define ATTR_NOTHROW
#define ATTR_NONNULL

#define ATTR_PURE
#define ATTR_CONST
#define ATTR_COLD
#define ATTR_HOT

#define ATTR_FORMAT(s,c)        

#define ATTR_EXPECT_TRUE(x)     (x)
#define ATTR_EXPECT_FALSE(x)    (x)
#endif

#define ATTR_H
#endif

