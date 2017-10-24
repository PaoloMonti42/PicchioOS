#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <stdarg.h>

char curses_init ();

void curses_log (FILE *out, int color, const char *fmt, va_list argp);

void curses_notify ();

int curses_getTeamsForGame ();

void curses_monitorMaster ();

void curses_destroyUI ();

#endif
