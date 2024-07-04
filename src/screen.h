#ifndef SCREEN_H

#include "err.h"

typedef struct Screen {
    struct {
        unsigned int x;
        unsigned int y;
    } dims;
} Screen;

ErrDecl screen_dims(Screen *screen);
void screen_enter(void); // smcup
void screen_leave(void); // rmcup

#define SCREEN_H
#endif

