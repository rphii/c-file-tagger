#ifndef PLATFORM_H

#include <rlc/platform-detect.h>
#include <rlso.h>

#define ERR_platform_colorprint_init(...) "failed enabling color prints"
ErrDecl platform_colorprint_init(void);

#define ERR_platform_fmt_home(...) "failed getting user home directory"
ErrDecl platform_fmt_home(So *str);

#define ERR_platform_fmt_cwd(...) "failed getting current working directory"
ErrDecl platform_fmt_cwd(So *str);

void platform_path_up(So *path);

#define ERR_platform_expand_path(...) "failed expanding path"
ErrDecl platform_expand_path(So *path, const So *base, const So *home);

#define ERR_platform_file

int platform_getch(void);
void platform_clear(void);
void platform_trace(void);

#define PLATFORM_H
#endif

