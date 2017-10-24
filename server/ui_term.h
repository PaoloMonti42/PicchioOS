#ifndef UI_TERM_H
#define UI_TERM_H

#include <stdarg.h>

char term_init ();

void term_log (FILE *out, int color, const char *fmt, va_list argp);

void term_notify ();

int term_getTeamsForGame ();

void term_monitorMaster ();

void term_destroyUI ();

#endif
