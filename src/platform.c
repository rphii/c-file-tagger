#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
//#include "colorprint.h"
#include <rphii/str.h>

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
    str_copy(str, str_ensure_dir(str_l(homedir)));
    //str_rremove_ch(str, PLATFORM_CH_SUBDIR, '\\'); // juuuust to be sure
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
        str_copy(str, str_ensure_dir(str_l(cwd)));
    } else {
        THROW("getcwd() error");
    }
    //str_rremove_ch(str, PLATFORM_CH_SUBDIR, '\\'); // juuuust to be sure
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
        n = str_rfind_ch(path2, PLATFORM_CH_SUBDIR);
        if(n == 0 || n >= str_len(path2)) {
            path2.len = 0;
            break;
        }
        path2.len = n - 1;
        path2 = str_ensure_dir(path2);
        //rstr_rremove_ch(&path2, PLATFORM_CH_SUBDIR, '\\');
        if(str_len(path2) && str_at(path2, str_len(path2)-1) == PLATFORM_CH_SUBDIR) {
            --path2.len;
        } else {
            path2.len = n;
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


#if 1 /* STR_EXPAND_PATH ??? -> move to file stuff ... {{{*/
#include <rphii/file.h>
ErrDecl platform_expand_path(Str *path, const Str *base, const Str *home) // TODO: move into platform.c ... {{{
{
    ASSERT_ARG(path);
    ASSERT_ARG(base);
    ASSERT_ARG(home);
    int err = 0;
    Str result = {0};
    Str temp = {0};
    Str base2 = *base;
    str_trim(*path);
#if defined(PLATFORM_WINDOWS)
    ABORT("not yet implemented in windows");
#else
    if(!str_len(*path)) return 0;
    if(str_len(*path) >= 2 && !str_cmp(str_iE(*path, 2), STR("~/"))) {
        str_fmt(&result, "%.*s%.*s", STR_F(*home), STR_F(str_i0(*path, 1)));
        /* assign result */
        str_clear(path);
        temp = *path;
        *path = result;
        result = temp;
    } else if(str_at(*path, 0) != PLATFORM_CH_SUBDIR) {
        if(file_get_type(base2) != FILE_TYPE_DIR) {
            platform_path_up(&base2);
        }
        //printff("%.*s .. %.*s", STR_F(&base2), STR_F(path));
        if(str_len(base2)) {
            str_fmt(&result, "%.*s%c%.*s", STR_F(base2), PLATFORM_CH_SUBDIR, STR_F(*path));
        } else {
            str_fmt(&result, "%.*s", STR_F(*path));
        }
        /* assign result */
        str_clear(path);
        temp = *path;
        *path = result;
        result = temp;
    }
    /* remove any and all dot-dot's -> '..' */
    for(;;) {
        size_t n = str_find_substr(*path, STR("../"), false);
        if(n >= str_len(*path)) break;
        Str prepend = *path;
        Str append = *path;
        prepend.len = n;
        append.str = append.str + n + str_len(STR(".."));
        prepend = str_ensure_dir(prepend);
        //rstr_rremove_ch(&prepend, PLATFORM_CH_SUBDIR, '\\');
        platform_path_up(&prepend);
        str_clear(&result);
        str_fmt(&result, "%.*s%.*s", STR_F(prepend), STR_F(append));
        temp = *path;
        *path = result;
        result = temp;
    }
    /* remove any and all folder-dot-folders -> /./ */
    for(;;) {
        size_t n = str_find_substr(*path, STR("./"), false);
        if(n >= str_len(*path)) break;
        Str prepend = *path;
        Str append = *path;
        prepend.len = n;
        append.str = append.str + n + str_len(STR("."));
        prepend = str_ensure_dir(prepend);
        //rstr_rremove_ch(&prepend, PLATFORM_CH_SUBDIR, '\\');
        str_fmt(&result, "%.*s%.*s", STR_F(prepend), STR_F(append));
        temp = *path;
        *path = result;
        result = temp;
    }
#endif
clean:
    str_free(&temp);
    return err;
error:
    ERR_CLEAN;
} /*}}}*/
#endif /*}}}*/


/******************************************************************************/
/* clear - terminal clearing **************************************************/
/******************************************************************************/

void platform_clear(void)
{
    printf("\033[H\033[J""\033[H\033[J");
}

