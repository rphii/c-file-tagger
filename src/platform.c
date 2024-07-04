#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "colorprint.h"
#include "err.h"

#define S(x)        #x
#define S_(x)       S(X)

int platform_colorprint_init(void)
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

/******************************************************************************/
/* getch **********************************************************************/
/******************************************************************************/

#if defined(PLATFORM_WINDOWS)
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

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


