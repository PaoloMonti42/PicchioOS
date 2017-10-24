#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"
#include "ui.h"

const char * const term_colors [] = {
    "\x1B[0m",
    "\x1B[31m",
    "\x1B[32m",
    "\x1B[33m",
    "\x1B[34m",
    "\x1B[35m",
    "\x1B[36m",
    "\x1B[37m"
};

void intHandler (int signo) {
    game.state = GAM_STOP;
    signal(SIGINT, SIG_DFL);
}

char term_init () {
    /* TODO: refuse if window is too small */

    return 1;
}

void term_log (FILE *out, int color, const char *fmt, va_list argp) {
    char buffer[LOGLINESIZE];

    int size = vsnprintf (buffer, LOGLINESIZE, fmt, argp);

    if (out != NULL)
        write (fileno (out), buffer, size);

    printf ("%s%s", term_colors[color], buffer);
    printf (RESET);
    fflush (stdout);
}

void term_notify () { }

int term_getTeamsForGame () {
    int i;
    char buf[MAXTEAM * 3 + 1];

    char invalidInput;
    int rankCmp;

    /* print all teams */
    printf ("   +--------------------------------------+\n");
    printf ("   |" "\x1B[31m" " TEAMS " RESET "                               |\n");
    printf ("   +--------------------------------------+\n");
    for (i=0; i<game.nbTeams; i++)
        if (game.teams[i].robotType != RBT_MISS)
            printf ("   | %2d: %s%-" STR(MAXNAMESIZE) "s " RESET " |\n", 
                    i,
                    term_colors[COL(i)],
                    game.teams[i].name);
    printf ("   +--------------------------------------+\n");

    /* prompt for game composition */
    printf ("Which teams are going to participate? (^D to end the contest)\n");

    do {
        char *p;
        for (i=0; i<game.nbTeams; i++) {
            game.teams[i].active = 0;
        }
        game.leaders[0] = 0xFF;
        game.leaders[1] = 0xFF;

        invalidInput = 0;

        printf ("> ");
        fflush (stdout);
        p = fgets (buf, 4 * 3 + 1, stdin);

        if (p == NULL)
            return 0;

        rankCmp = 0;
        /* get participating teams */
        for (i=-1; *p && *p != '\n'; p++) {
            if (*p == ' ') {
                if (i != -1) {
                    if (i < game.nbTeams && game.teams[i].robotType != RBT_MISS && !game.teams[i].active) {
                        game.teams[i].active = 1;
                        game.teams[i].side = rankCmp/2;
                        game.teams[i].role = rankCmp%2;
                        if (rankCmp % 2 == 0) {
                            game.leaders[rankCmp/2] = i;
                        }
                        rankCmp++;
                    } else {
                        invalidInput = 1;
                        printf ("Invalid team number: %d\n", i);
                    }
                    i = -1;
                }
            } else {
                if (*p < '0' || *p > '9') {
                    invalidInput = 1;
                    printf ("Invalid input number: '%c'\n", *p);
                    break;
                }

                if (i == -1)
                    i = *p - '0';
                else
                    i = i*10 + *p - '0';
            }
        }

        if (i != -1) {
            if (i < game.nbTeams && game.teams[i].robotType != RBT_MISS && !game.teams[i].active) {
                game.teams[i].active = 1;
                game.teams[i].side = rankCmp/2;
                game.teams[i].role = rankCmp%2;
                if (rankCmp % 2 == 0) {
                    game.leaders[rankCmp/2] = i;
                }
                rankCmp++;
            } else {
                invalidInput = 1;
                printf ("Invalid team number: %d\n", i);
            }
        }

        if (rankCmp != 2 && rankCmp != 4) {
            printf ("You should select either 2 or 4 teams.\n");
            invalidInput = 1;
        }
    } while (invalidInput);

    return rankCmp;
}

void term_monitorMaster () {
    /* catch SIGINT to stop game */
    if (signal (SIGINT, intHandler) == SIG_ERR) {
        fprintf (stderr, "Couldn't catch SIGINT.\n");
        exit (EXIT_FAILURE);
    }
}

void term_destroyUI () {

}
