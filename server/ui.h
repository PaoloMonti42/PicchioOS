#ifndef UI_H
#define UI_H

#include <stdarg.h>

#include "server.h"

#define KNRM            0
#define KRED            1
#define KGRN            2
#define KYEL            3
#define KBLU            4
#define KMAG            5
#define KCYN            6
#define KWHT            7

#define NBCOLORS        5

extern int const playerColors [NBCOLORS];

#define COL(i) playerColors[(i) % NBCOLORS]

struct observerMethods {
    void (*__log) (FILE *, int, const char *, va_list); 
    void (*notify) ();
    int (*getTeamsForGame) ();
    void (*monitorMaster) ();
    void (*destroyUI) ();
};

char init_gui (char curses, struct observerMethods *GUI);

void __mylog (int color, const char *fmt, ...);
void debug (int lvl, int color, const char *fmt, ...);

#define LOGLINESIZE         150

#endif
