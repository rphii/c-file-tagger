#include <stdlib.h>

#include "platform.h"
#include "screen.h"
#include "info.h"

#if defined(PLATFORM_LINUX)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

int screen_dims(Screen *screen)
{
#if defined(PLATFORM_LINUX)
    struct winsize w;
    TRY(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w), "could not get screen sizes");
    screen->dims.x = w.ws_row;
    screen->dims.y = w.ws_row;
    return 0;
error:
    return -1;
#else
    return 0;
#endif
}

/* I don't really care if those functions fail */
void screen_enter(void)
{
#if defined(PLATFORM_LINUX)
    printf("\033[?25l"); // hide cursor
    int result = system("tput smcup");
    if(result) info(syscmd_failed, "failed system command: 'tput smcup' (enter alternate screen)");
#endif
}

void screen_leave(void)
{
#if defined(PLATFORM_LINUX)
    printf("\033[?25h"); // show cursor
    int result = system("tput rmcup");
    if(result) info(syscmd_failed, "failed system command: 'tput smcup' (exit alternate screen)");
#endif
}

