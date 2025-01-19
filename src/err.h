#ifndef ERR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "colorprint.h"
#include "attr.h"

#ifndef DEBUG
#define DEBUG           0
#define DEBUG_GETCH     0
#define DEBUG_GETCHAR   0
#define DEBUG_INSPECT   0
#define DEBUG_DISABLE_ERR_MESSAGES  0
#endif

#define SIZE_IS_NEG(x)      (((SIZE_MAX >> 1) + 1) & (x))

#ifndef SSIZE_MIN
#define SSIZE_MIN       ((ssize_t)1 << (8*sizeof(ssize_t)-1))
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX           ~((ssize_t)1 << (8*sizeof(ssize_t)-1))
#endif

#define SIZE_ARRAY(x)       (sizeof(x)/sizeof(*x))

/* use this when declaring function that can error */
#define ErrDecl             ATTR_NODISCARD int
#define ErrImpl             ATTR_NODISCARD inline int 
#define ErrImplStatic       ATTR_NODISCARD static inline int 
#define ErrDeclStatic       ATTR_NODISCARD static inline int

#define ERR_STRINGIFY(S)    #S
#define ERR_CLEAN           do { err = -1; goto clean; } while(0)

/* general error messages */
#define ERR_INTERNAL(msg)   "internal error: " msg
#define ERR_MALLOC          "failed to malloc"
#define ERR_CREATE_POINTER  "expected pointer to Create"
#define ERR_SIZE_T_POINTER  "expected pointer to size_t"
#define ERR_BOOL_POINTER    "expected pointer to bool"
#define ERR_CSTR_POINTER    "expected pointer to c-string"
#define ERR_CSTR_INVALID    "encountered unexpected null character in c-string"
#define ERR_UNHANDLED_ID    "unhandled id"
#define ERR_UNREACHABLE     "unreachable error"
#define ERR_UNIMPLEMENTED   "unimplemented"
#define ERR_NULL_ARG        "unexpected null pointer argument received"
#define ERR_SYSTEM          "failed executing system command"
#define ERR_POPEN           ERR_SYSTEM


#define ERR_FILE_STREAM     stderr

#if DEBUG_DISABLE_ERR_MESSAGES
#define ERR_PRINTF(fmt, ...)    {}
#else
//#define ERR_PRINTF(fmt, ...)    do { fflush(stdout); /* for whatever reason I have to do this fflush... */ fprintf(ERR_FILE_STREAM, fmt, ##__VA_ARGS__); } while(0)
#define ERR_PRINTF(fmt, ...)    do { fprintf(ERR_FILE_STREAM, fmt, ##__VA_ARGS__); } while(0)
#endif

/* macros */

#define THROW_PRINT(fmt, ...)   do { \
    ERR_PRINTF(F("[ERROR]", BOLD FG_RD_B) " " F("%s:%d:%s", FG_WT_B) " " fmt "" , __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)

#define THROW(fmt, ...)      do { \
    THROW_PRINT(fmt "\n", ##__VA_ARGS__); \
    goto error; } while(0)

#define THROW_P(print_err, fmt, ...)    do { \
        if(print_err) { \
            THROW(fmt, ##__VA_ARGS__); \
        } else { \
            goto error; \
        } \
    } while(0)

#define ABORT(fmt, ...)      do { \
    ERR_PRINTF(F("[ABORT]", BOLD FG_BK BG_RD_B) " " F("%s:%d:%s (end of trace)", FG_WT_B) " " fmt "\n" , __FILE__, __LINE__, __func__, ##__VA_ARGS__); exit(-1); } while(0)

#define INFO(fmt, ...)       do { \
        ERR_PRINTF(F("[INFO]", BOLD FG_YL_B) " " F("%s:%d:%s", FG_WT_B) " " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)

#define INFO_REDUCED(fmt, ...)  do { \
        ERR_PRINTF(F("[INFO]", BOLD FG_YL_B) " " fmt "\n", ##__VA_ARGS__); \
    } while(0)


#ifndef NDEBUG
#define ASSERT(stmt, fmt, ...)   do { \
    if (!(stmt)) { \
        ABORT("assertion of '" ERR_STRINGIFY(stmt) "' failed... " fmt, ##__VA_ARGS__); } \
    } while(0)
#else
#define ASSERT(stmt, fmt, ...)   do { } while(0)
#endif

#define ASSERT_ERROR(x)                     assert(0 && (x))

#define ASSERT_ARG(arg)     ASSERT(arg, ERR_NULL_ARG)

#define printff(fmt, ...)   do { \
        printf(fmt, ##__VA_ARGS__); \
        printf(F(" * ", FG_BL_B) F("%s:%i \n", FG_BK_B) , __func__, __LINE__); \
    } while(0);


#define TRY_BASE(stmt, fmt, ...)                do { if (stmt) { THROW(fmt, ##__VA_ARGS__); } } while(0)
#define TRY_CONST(function)                     do { if (function) { THROW(ERR_##function); } } while(0)
#define TRY_CALL(function)                      do { if (function) { THROW("%s", ERR_##function); } } while(0)
//#define TRY_CONST(function)                     TRY_BASE(function, ERR_##function)           // try; err is const-defined-string
//#define TRY_CALL(function)                      TRY_BASE(function, "%s", ERR_##function)     // try; err is generic-string
                                                                                                     //
#define TRY_BASE_P(print_err, stmt, fmt, ...)   do { if (stmt) { THROW_P(print_err, fmt, ##__VA_ARGS__); } } while(0)
#define TRY_CONST_P(print_err, function)        TRY_BASE_P(print_err, function, ERR_##function)
#define TRY_CALL_P(print_err, function)         TRY_BASE_P(print_err, function, "%s", ERR_##function)     // try; err is generic-string

#define TRY(stmt, fmt, ...)                     do { if (stmt) { THROW(fmt, ##__VA_ARGS__); } } while(0)
#define TRYC(function)                          TRY(function, ERR_##function)           // try; err is const-defined-string
#define TRYG(function)                          TRY(function, "%s", ERR_##function)     // try; err is generic-string

#define TRY_P(print_err, stmt, fmt, ...)        do { if (stmt) { THROW_P(print_err, fmt, ##__VA_ARGS__); } } while(0)
#define TRYC_P(print_err, function)             TRY_P(print_err, function, ERR_##function)
#define TRYG_P(print_err, function)             TRY_P(print_err, function, "%s", ERR_##function)     // try; err is generic-string

#define ERR_H
#endif

