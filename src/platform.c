#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
//#include "colorprint.h"
#include <rlso.h>

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

ErrDecl platform_fmt_home(So *str)
{
    ASSERT_ARG(str);
#if defined(PLATFORM_WINDOWS)
    THROW("not yet implemented for windows");
#else
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    so_copy(str, so_ensure_dir(so_l(homedir)));
    //so_rremove_ch(str, PLATFORM_CH_SUBDIR, '\\'); // juuuust to be sure
#endif
    return 0;
error:
    return -1;
}

ErrDecl platform_fmt_cwd(So *str)
{
    ASSERT_ARG(str);
#if defined(PLATFORM_WINDOWS)
    THROW("not yet implemented for windows");
#else
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        so_copy(str, so_ensure_dir(so_l(cwd)));
    } else {
        THROW("getcwd() error");
    }
    //so_rremove_ch(str, PLATFORM_CH_SUBDIR, '\\'); // juuuust to be sure
#endif
   return 0;
error:
   return -1;
}

void platform_path_up(So *path) //{{{
{
    ASSERT_ARG(path);
    So path2 = *path;
#if defined(PLATFORM_WINDOWS)
    ABORT("not yet implemented for windows");
#else
    size_t n = 0;
    for(;;) {
        n = so_rfind_ch(path2, PLATFORM_CH_SUBDIR);
        if(n == 0 || n >= so_len(path2)) {
            path2.len = 0;
            break;
        }
        path2.len = n - 1;
        path2 = so_ensure_dir(path2);
        //rso_rremove_ch(&path2, PLATFORM_CH_SUBDIR, '\\');
        if(so_len(path2) && so_at(path2, so_len(path2)-1) == PLATFORM_CH_SUBDIR) {
            --path2.len;
        } else {
            path2.len = n;
            break;
        }
    }
#endif
    *path = path2;
    //printff("PATH2=[%.*s]", SO_F(&path2));
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
ErrDecl platform_expand_path(So *path, const So *base, const So *home) // TODO: move into platform.c ... {{{
{
    ASSERT_ARG(path);
    ASSERT_ARG(base);
    ASSERT_ARG(home);
    int err = 0;
    So result = {0};
    So temp = {0};
    So base2 = *base;
    so_trim(*path);
#if defined(PLATFORM_WINDOWS)
    ABORT("not yet implemented in windows");
#else
    if(!so_len(*path)) return 0;
    if(so_len(*path) >= 2 && !so_cmp(so_iE(*path, 2), so("~/"))) {
        so_fmt(&result, "%.*s%.*s", SO_F(*home), SO_F(so_i0(*path, 1)));
        /* assign result */
        so_clear(path);
        temp = *path;
        *path = result;
        result = temp;
    } else if(so_at(*path, 0) != PLATFORM_CH_SUBDIR) {
        if(so_file_get_type(base2) != SO_FILE_TYPE_DIR) {
            platform_path_up(&base2);
        }
        //printff("%.*s .. %.*s", SO_F(&base2), SO_F(path));
        if(so_len(base2)) {
            so_fmt(&result, "%.*s%c%.*s", SO_F(base2), PLATFORM_CH_SUBDIR, SO_F(*path));
        } else {
            so_fmt(&result, "%.*s", SO_F(*path));
        }
        /* assign result */
        so_clear(path);
        temp = *path;
        *path = result;
        result = temp;
    }
    /* remove any and all dot-dot's -> '..' */
    for(;;) {
        size_t n = so_find_sub(*path, so("../"), false);
        if(n >= so_len(*path)) break;
        So prepend = *path;
        So append = *path;
        prepend.len = n;
        append.str = append.str + n + so_len(so(".."));
        prepend = so_ensure_dir(prepend);
        //rso_rremove_ch(&prepend, PLATFORM_CH_SUBDIR, '\\');
        platform_path_up(&prepend);
        so_clear(&result);
        so_fmt(&result, "%.*s%.*s", SO_F(prepend), SO_F(append));
        temp = *path;
        *path = result;
        result = temp;
    }
    /* remove any and all folder-dot-folders -> /./ */
    for(;;) {
        size_t n = so_find_sub(*path, so("./"), false);
        if(n >= so_len(*path)) break;
        So prepend = *path;
        So append = *path;
        prepend.len = n;
        append.str = append.str + n + so_len(so("."));
        prepend = so_ensure_dir(prepend);
        //rso_rremove_ch(&prepend, PLATFORM_CH_SUBDIR, '\\');
        so_fmt(&result, "%.*s%.*s", SO_F(prepend), SO_F(append));
        temp = *path;
        *path = result;
        result = temp;
    }
#endif
clean:
    so_free(&temp);
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

