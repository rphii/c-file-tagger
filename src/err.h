#ifndef ERR_H

#include <stddef.h>
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

/* use this when declaring function that can error */
#define ErrDecl             ATTR_NODISCARD int
#define ErrDeclStatic       ATTR_NODISCARD static inline int

#define SIZE_ARRAY(x)       (sizeof(x)/sizeof(*x))

#define ERR_STRINGIFY(S)    #S
#define ERR_CLEAN           do { err = -1; goto clean; } while(0)

/* general error messages */
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

void screen_leave(void);    /* implementation is in "screen.h" */

void info_handle_abort(void);
void platform_trace(void);  /* implementation in platform.c */

/* macros */

#define THROW(fmt, ...)      do { \
    (void)screen_leave(); \
    info_handle_abort(); \
    ERR_PRINTF(F("[ERROR]", BOLD FG_RD_B) " " F("%s:%d:%s", FG_WT_B) " " fmt "\n" , __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    goto error; } while(0)

#define ABORT(fmt, ...)      do { \
    (void)screen_leave(); \
    info_handle_abort(); \
    platform_trace(); ERR_PRINTF(F("[ABORT]", BOLD FG_BK BG_RD_B) " " F("%s:%d:%s (end of trace)", FG_WT_B) " " fmt "\n" , __FILE__, __LINE__, __func__, ##__VA_ARGS__); exit(-1); } while(0)

#define INFO(fmt, ...)       do { \
        /*_Static_assert(0, "don't use");*/\
        /*printff(fmt, ##__VA_ARGS__); */\
        ERR_PRINTF(F("[INFO]", BOLD FG_YL_B) " " F("%s:%d:%s", FG_WT_B) " " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)

#define INFO_REDUCED(fmt, ...)  do { \
        ERR_PRINTF(F("[INFO]", BOLD FG_YL_B) " " fmt "\n", ##__VA_ARGS__); \
    } while(0)

#define TRY(stmt, fmt, ...)  if (stmt) { THROW(fmt, ##__VA_ARGS__); }
#define ASSERT_ERROR(x)      assert(0 && (x))

#ifndef NDEBUG
#define ASSERT(stmt, fmt, ...)   do { \
    if (!(stmt)) { \
        (void)screen_leave(); \
        info_handle_abort(); \
        /*platform_trace();*/ \
        ABORT("assertion of '" ERR_STRINGIFY(stmt) "' failed... " fmt, ##__VA_ARGS__); } \
    } while(0)
#else
#define ASSERT(stmt, fmt, ...)   do { } while(0)
#endif

#define ASSERT_ARG(arg)     ASSERT(arg, ERR_NULL_ARG)

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#define printff(fmt, ...)   do { /*printf("%s():%i: ", __func__, __LINE__);*/  \
        \
        int l = snprintf(0, 0, "* %s:%i ", __func__, __LINE__); \
        struct winsize w; \
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); \
        int printed = printf(fmt, ##__VA_ARGS__); \
        int missing = w.ws_col - (printed + l) % w.ws_col; \
        if(0){\
        for(int i = 0; i < missing; ++i) { \
            if(!((missing - i)%2)) printf(F(".", FG_BK_B));\
            else printf(" ");\
        } \
        }else{printf("%*s", missing, "");}\
        printf("* %s:%i \n", __func__, __LINE__); \
        \
    } while(0);

//#define TRYF(function, ...)  TRY(function(__VA_ARGS__), function##_ERR(__VA_ARGS__))
#define TRYC(function)      TRY(function, ERR_##function)           // try; err is const-defined-string
#define TRYG(function)      TRY(function, "%s", ERR_##function)     // try; err is generic-string


#define ERR_H
#endif

