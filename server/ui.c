#include "ui.h"
#include "ui_term.h"
#include "ui_curses.h"

int const playerColors [] = {
    KGRN,
    KYEL,
    KBLU,
    KMAG,
    KCYN
};

inline void __mylog (int color, const char *fmt, ...) {
    va_list argp;

    va_start (argp, fmt);

    (*GUI.__log) (logFile, color, fmt, argp);

    va_end (argp);
}

inline void debug (int lvl, int color, const char *fmt, ...) {
    va_list argp;

    if (lvl > debugLvl)
        return;

    va_start (argp, fmt);

    (*GUI.__log) (logFile, color, fmt, argp);

    va_end (argp);
}

char init_gui (char curses, struct observerMethods *GUI) {
    char c;
    if (curses) {
        if ((c = curses_init ()) == 0)
            return 0;
        GUI->__log = &curses_log;
        GUI->notify = &curses_notify;
        GUI->getTeamsForGame = &curses_getTeamsForGame;
        GUI->monitorMaster = &curses_monitorMaster;
        GUI->destroyUI = &curses_destroyUI;
    } else {
        if ((c = term_init ()) == 0)
            return 0;
        GUI->__log = &term_log;
        GUI->notify = &term_notify;
        GUI->getTeamsForGame = &term_getTeamsForGame;
        GUI->destroyUI = &term_destroyUI;
    }

    return c;
}
