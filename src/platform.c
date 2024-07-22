#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
//#include "colorprint.h"
#include "err.h"
#include "str.h"

#if defined(PLATFORM_WINDOWS)
#include <conio.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <termios.h>
#include <linux/limits.h>
#endif

#define S(x)        #x
#define S_(x)       S(X)

ErrDecl platform_colorprint_init(void)
{
    int err = 0;
#if defined(PLATFORM_WINDOWS) && !defined(COLORPRINT_DISABLE)
    err = system("chcp 65001 >nul");
    if(err) {
        THROW("failed enabling utf-8 codepage. Try compiling with -D" S(PLATFORM_DISABLE) "");
    }
clean:
    return err;
error:
    ERR_CLEAN;
#else
    return err;
#endif
}

ErrDecl platform_fmt_home(Str *str)
{
    ASSERT_ARG(str);
#if defined(PLATFORM_WINDOWS)
    THROW("not yet implemented for windows");
#else
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    TRYC(str_fmt(str, "%s", homedir));
    str_remove_trailing_ch(str, PLATFORM_CH_SUBDIR, '\\'); // juuuust to be sure
#endif
    return 0;
error:
    return -1;
}

ErrDecl platform_fmt_cwd(Str *str)
{
    ASSERT_ARG(str);
#if defined(PLATFORM_WINDOWS)
    THROW("not yet implemented for windows");
#else
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        TRYC(str_fmt(str, "%s", cwd));
    } else {
        THROW("getcwd() error");
    }
    str_remove_trailing_ch(str, PLATFORM_CH_SUBDIR, '\\'); // juuuust to be sure
#endif
   return 0;
error:
   return -1;
}

void platform_path_up(Str *path) //{{{
{
    ASSERT_ARG(path);
    Str path2 = *path;
#if defined(PLATFORM_WINDOWS)
    ABORT("not yet implemented for windows");
#else
    size_t n = 0;
    for(;;) {
        n = str_rch(&path2, PLATFORM_CH_SUBDIR, 0);
        if(n == 0 || n >= str_length(&path2)) {
            path2.last = path2.first;
            break;
        }
        path2.last = path2.first + n - 1;
        str_remove_trailing_ch(&path2, PLATFORM_CH_SUBDIR, '\\');
        if(str_length(&path2) && str_get_back(&path2) == PLATFORM_CH_SUBDIR) {
            --path2.last;
        } else {
            path2.last = path2.first + n;
            break;
        }
    }
#endif
    *path = path2;
    //printff("PATH2=[%.*s]", STR_F(&path2));
} //}}}

/******************************************************************************/
/* getch **********************************************************************/
/******************************************************************************/

int platform_getch(void)
{
    //printf(F("[press any key] ", IT FG_CY_B));
    fflush(stdout);

#if defined(PLATFORM_WINDOWS)

    return _getch();

#else

    int buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0) {
        perror("tcsetattr()");
    }
    old.c_lflag &= (tcflag_t)~ICANON;
    old.c_lflag &= (tcflag_t)~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }
    if(read(0, &buf, 1) < 0) {
        perror("read()");
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }
    //printf("%c", buf);
    return buf;

#endif
}

#if defined(PLATFORM_LINUX)

#include <execinfo.h>
#include <stdio.h>

#endif

void platform_trace(void)
{
#if defined(PLATFORM_LINUX)
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        printf("%s\n", strs[i]);
    }
    free(strs);
#endif
}


/******************************************************************************/
/* clear - terminal clearing **************************************************/
/******************************************************************************/

void platform_clear(void)
{
    printf("\033[H\033[J""\033[H\033[J");
}

