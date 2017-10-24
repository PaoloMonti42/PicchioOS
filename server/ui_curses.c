#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "server.h"
#include "ui.h"

struct linePart {
    struct linePart *next;
    int color;
    int length;
    char content[LOGLINESIZE];
};

struct outputLine {
    struct outputLine *next;
    struct outputLine *previous;
    struct linePart *firstPart;
    struct linePart *lastPart;
};

struct outputLine *first;
struct outputLine *last;
int nbLines;

#define TOTLINES        100

WINDOW *logwin, *teamwin, *helpwin;
pthread_mutex_t ui_lock;

int const curses_colors [] = {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};

WINDOW * create_window (int height, int width, int starty, int startx) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);
    return local_win;
}

int rankSelect = -1;
int teamFromRank [MAXTEAM];

int colWidth;

const char * rankAsString[] = {
    "R B",
    "R F",
    "L B",
    "L F"
};

int rankFromActiveTeam(struct team *team) {
    if (!team->active && !team->kicked && !team->ended)
        return -1;
    return team->side*2 + (&game.teams[game.leaders[team->side]] != team);
}

void curses_notify () {
    int i, max=0;

    pthread_mutex_lock (&ui_lock);

    werase (teamwin);
    wbkgd(teamwin,COLOR_PAIR(1));
    wattron(teamwin, COLOR_PAIR(1));

    for (i=0; i<game.nbTeams; i++)
        if (game.teams[i].active || game.teams[i].kicked || game.teams[i].ended)
            max ++;

    for (i=0; i<game.nbTeams; i++) {
        int bgCol = 1;
        int l, j;

        if (game.teams[i].connected)
            wattron (teamwin, A_BOLD);

        if (game.teams[i].active || game.teams[i].kicked || game.teams[i].ended) {
            if (rankFromActiveTeam(&game.teams[i]) == rankSelect)
                bgCol = 10;
            wattron (teamwin, COLOR_PAIR (bgCol+0));
            wmove (teamwin, rankFromActiveTeam(&game.teams[i]), 0);
            teamFromRank [rankFromActiveTeam(&game.teams[i])] = i;
            wprintw (teamwin, " %s ", rankAsString[rankFromActiveTeam(&game.teams[i])]);
        } else {
            if (max == rankSelect)
                bgCol = 10;
            wattron (teamwin, COLOR_PAIR (bgCol+0));
            teamFromRank [max] = i;
            wmove (teamwin, max++, 0);
            wprintw (teamwin, "     ");
        }

        wattron (teamwin, COLOR_PAIR (bgCol+COL(i)));
        wprintw (teamwin, "%s", game.teams[i].name);

        l = colWidth - strlen (game.teams[i].name) - 16;
        for (j=0; j<l; j++)
            wprintw (teamwin, " ");

        wattron (teamwin, COLOR_PAIR (bgCol+0));
        wprintw (teamwin, "     ");

        if (game.teams[i].kicked)
            wprintw (teamwin, " KCK  ");
        else if (game.teams[i].ended)
            wprintw (teamwin, " END  ");
        else
            wprintw (teamwin, "      ");

        wattroff (teamwin, A_BOLD);
    }

    wrefresh(teamwin);

    pthread_mutex_unlock (&ui_lock);
}

void log_refresh ();
void helpwin_refresh ();

char window_init () {
    WINDOW *sepwin;
    int i;

    if (LINES < game.nbTeams+11 || COLS < MAXNAMESIZE+16 + colWidth) {
        endwin ();
        fprintf (stderr, "Window is too small! (%dx%d)\n", LINES, COLS);
        pthread_mutex_unlock (&ui_lock);
        return 0;
    }

    logwin = create_window (LINES-1, COLS-colWidth-1, 0, 0);
    teamwin = create_window (game.nbTeams+1, colWidth, 0, COLS-colWidth);
    helpwin = create_window (LINES-game.nbTeams-2, colWidth, game.nbTeams+1, COLS-colWidth);
    sepwin = create_window(LINES-1, 1, 0, COLS-colWidth-1);
    for (i=0; i<LINES-1; i++)
        mvwprintw (sepwin, i, 0, "|");

    keypad(teamwin, TRUE);
    curs_set(0);

    wbkgd(sepwin,COLOR_PAIR(10));
    wrefresh(sepwin);

    wbkgd(helpwin,COLOR_PAIR(10));
    wrefresh(helpwin);

    wbkgd(logwin,COLOR_PAIR(1));
    wrefresh(logwin);

    pthread_mutex_unlock (&ui_lock);

    helpwin_refresh ();
    curses_notify ();
    log_refresh ();

    return 1;
}

void handle_winch (int sig) {
    pthread_mutex_lock (&ui_lock);

    endwin ();
    clear ();
    refresh ();

    window_init ();
}

char curses_init () {
    int i;
    struct sigaction sa;

    if (initscr() == NULL) {
        fprintf (stderr, "Error initialising ncurses.\n");
        return 0;
    }

    pthread_mutex_lock (&ui_lock);

    start_color();
    noecho();
    cbreak();

    colWidth = 16+MAXNAMESIZE;
    colWidth = colWidth > 45 ? colWidth : 45;

    first = (struct outputLine *) malloc (sizeof (struct outputLine));
    last = first;
    first->firstPart = NULL;
    first->lastPart = NULL;
    nbLines = 1;

    init_color(COLOR_WHITE, 1000, 1000, 1000);

    for (i=0; i<8; i++) {
        init_pair (i+1, curses_colors[i], COLOR_WHITE);
        if (i!=0)
            init_pair (i+10, curses_colors[i], COLOR_BLACK);
    }
    
    init_pair(10, COLOR_WHITE, COLOR_BLACK);

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);

    return window_init ();
}

void printLine (int color, const char *content, char newLine) {
    size_t l = strlen (content);
    struct linePart *part = (struct linePart *) malloc (sizeof (struct linePart));
    part->next = NULL;
    part->length = l;
    part->color = color;
    strncpy (part->content, content, LOGLINESIZE-1);

    if (first->firstPart == NULL) {
        first->firstPart = part;
        first->lastPart = part;
    } else {
        first->lastPart->next = part;
        first->lastPart = part;
    }

    if (newLine) {
        struct outputLine *prevFirst = first;
        first = (struct outputLine *) malloc (sizeof (struct outputLine));
        first->next = prevFirst;
        prevFirst->previous = first;
        first->firstPart = NULL;
        first->lastPart = NULL;
        if (++nbLines > TOTLINES) {
            struct outputLine *prevLast = last;
            struct linePart *part, *nextPart;

            last = last->previous;

            for (part = prevLast->firstPart; part != NULL; part = nextPart) {
                nextPart = part->next;
                free (part);
            }

            free (prevLast);
            nbLines --;
        }
    }
}

size_t outputLineLength (struct outputLine *l) {
    struct linePart *part;
    size_t count = 0;

    for (part = l->firstPart; part != NULL; part = part->next) {
        count += strlen (part->content);
    }

    return count;
}

void log_refresh () {
    int curLine, curOutputLine;
    struct outputLine *l;

    pthread_mutex_lock (&ui_lock);

    werase (logwin);
    for (curOutputLine=0, curLine=LINES-2, l=first; curLine>=0 && curOutputLine<TOTLINES && curOutputLine<nbLines; curOutputLine++, l=l->next) {
        struct linePart *part;
        size_t lineLength;
        int curCol = 0;

        lineLength = outputLineLength (l);
        if (lineLength == 0) {
            curLine--;
            continue;
        }
        lineLength = (lineLength-1)/(COLS-colWidth-1)+1;
        curLine -= lineLength;
        lineLength = curLine+1;

        part = l->firstPart;
        while (part != NULL) {
            wmove (logwin, lineLength, curCol);

            /* FIXME: for the top line, if first part is split, last part will not show */
            if (lineLength >= 0) {
                wattron (logwin, COLOR_PAIR(1+part->color));
                wprintw (logwin, "%s", part->content);
            }

            if (strlen (part->content) <= COLS-colWidth-1-curCol) {
                curCol += strlen (part->content);
            } else {
                curCol = strlen (part->content) - (COLS-colWidth-1-curCol);
                lineLength++;
            }
            part = part->next;
        }
    }
    wrefresh(logwin);

    pthread_mutex_unlock (&ui_lock);
}

void curses_log (FILE *out, int color, const char *fmt, va_list argp) {
    char *p1, *p2;
    char buffer[LOGLINESIZE];

    int size = vsnprintf (buffer, LOGLINESIZE, fmt, argp);

    if (out != NULL)
        write (fileno (out), buffer, size);

    for (p1 = buffer; (p2 = strchr (p1, '\n')) != NULL; p1 = p2+1) {
        *p2 = '\0';
        printLine (curses_colors[color], p1, 1);
    }
    printLine (curses_colors[color], p1, 0);

    log_refresh ();
}

void setActiveTeamFromRank(unsigned char teamRank, int rank) {
    struct team *team = &game.teams[teamRank];
    team->side = rank/2;
    team->role = rank%2;
    if (rank % 2 == 0) {
        game.leaders[team->side] = teamRank;
    }
}

void helpwin_refresh () {
    pthread_mutex_lock (&ui_lock);
    wbkgd(helpwin,COLOR_PAIR(10));

    mvwprintw (helpwin, 1, 1, "     ");
    mvwprintw (helpwin, 2, 1, "                                 ");
    mvwprintw (helpwin, 3, 1, "                                  ");
    mvwprintw (helpwin, 4, 1, "                         ");
    mvwprintw (helpwin, 5, 1, "                           ");
    mvwprintw (helpwin, 6, 1, "                       ");
    mvwprintw (helpwin, 7, 1, "                              ");

    if (game.state == GAM_TEAM_SELECT) {
        mvwprintw (helpwin, 1, 1, "Help:");
        mvwprintw (helpwin, 2, 1, "        <ENTER> : Use these teams");
        mvwprintw (helpwin, 3, 1, "        <SPACE> : Toggle in/out");
        mvwprintw (helpwin, 4, 1, "        <NPAGE> : Move up");
        mvwprintw (helpwin, 5, 1, "        <PPAGE> : Move down");
        mvwprintw (helpwin, 6, 1, "              c : Clear");
        mvwprintw (helpwin, 7, 1, "              q : Quit contest");
    } else if (game.state == GAM_CONNECTING || game.state == GAM_RUNNING) {
        mvwprintw (helpwin, 1, 1, "Help:");
        mvwprintw (helpwin, 2, 1, "              k : Kick team");
        mvwprintw (helpwin, 3, 1, "              q : End current game");
    }

    wrefresh (helpwin);
    pthread_mutex_unlock (&ui_lock);
}

int curses_getTeamsForGame () {
    int rankCmp = 0;
    char end = 0;
    int i;
    rankSelect = 0;

    for (i=0; i<game.nbTeams; i++) {
        game.teams[i].active = 0;
        game.teams[i].kicked = 0;
        game.teams[i].ended = 0;
    }

    helpwin_refresh ();

    curses_notify ();

    while (!end) {
        int key = wgetch (teamwin);
        switch (key) {
            case '\n':
            case KEY_ENTER:
                if (rankCmp == 2 || rankCmp == 4)
                    end = 1;
                break;
            case KEY_DOWN:
                rankSelect = (rankSelect+1)%game.nbTeams;
                curses_notify ();
                break;
            case KEY_UP:
                if (--rankSelect < 0)
                   rankSelect += game.nbTeams;
                curses_notify ();
                break;
            case ' ':
                if (game.teams[teamFromRank[rankSelect]].active) {
                    int r = rankFromActiveTeam(&game.teams[teamFromRank[rankSelect]]);
                    game.teams[teamFromRank[rankSelect]].active = 0;
                    for (i=0; i<game.nbTeams; i++)
                        if (game.teams[i].active) {
                            int r2 = rankFromActiveTeam(&game.teams[i]);
                            if (r2 > r) {
                                setActiveTeamFromRank(i, r2-1);
                            }
                        }
                    rankCmp --;
                } else if (rankCmp < 4) {
                    setActiveTeamFromRank(teamFromRank[rankSelect], rankCmp++);
                    game.teams[teamFromRank[rankSelect]].active = 1;
                }

                curses_notify ();
                break;
            case KEY_NPAGE:
                if (rankSelect < rankCmp-1) {
                    int r = rankFromActiveTeam(&game.teams[teamFromRank[rankSelect]]);
                    setActiveTeamFromRank(teamFromRank[rankSelect+1], r);
                    setActiveTeamFromRank(teamFromRank[rankSelect], r+1);
                    rankSelect ++;
                    curses_notify ();
                }
                break;
            case KEY_PPAGE:
                if (rankSelect > 0 && game.teams[teamFromRank[rankSelect]].active) {
                    int r = rankFromActiveTeam(&game.teams[teamFromRank[rankSelect]]);
                    setActiveTeamFromRank(teamFromRank[rankSelect-1], r);
                    setActiveTeamFromRank(teamFromRank[rankSelect], r-1);
                    rankSelect --;
                    curses_notify ();
                }
                break;
            case 'c':
                for (i=0; i<game.nbTeams; i++)
                    game.teams[i].active = 0;
                rankSelect = 0;
                rankCmp = 0;
                curses_notify ();
                break;
            case 27:
            case 'q':
                return 0;
        }
    }

    rankSelect = -1;
    return rankCmp;
}

pthread_t tid;

void * threadMonitor (void * __dummy) {
    char end = 0;
    int i;

    rankSelect = 0;

    helpwin_refresh ();

    curses_notify ();

    while (!end) {
        int key = wgetch (teamwin);
        switch (key) {
            case KEY_DOWN:
                for (i=(rankSelect+1)%game.nbTeams; i != rankSelect; i=(i+1)%game.nbTeams)
                    if (game.teams[teamFromRank[i]].active)
                        break;

                if (i != rankSelect) {
                    rankSelect = i;
                    curses_notify ();
                }
                break;
            case KEY_UP:
                for (i=rankSelect-1; i != rankSelect; i--) {
                    if (i < 0)
                        i += game.nbTeams;
                    if (game.teams[teamFromRank[i]].active)
                        break;
                }

                if (i != rankSelect) {
                    rankSelect = i;
                    curses_notify ();
                }
                break;
            case 'k':
                sendKick (teamFromRank[rankSelect]);

                for (i=(rankSelect+1)%game.nbTeams; i != rankSelect; i=(i+1)%game.nbTeams)
                    if (game.teams[teamFromRank[i]].active)
                        break;

                if  (i == rankSelect) {
                    game.state = GAM_STOP;
                    pthread_mutex_destroy (&game.lock);
                    end = 1;
                } else
                    rankSelect = i;

                curses_notify ();

                break;
            case 'q':
                game.state = GAM_STOP;
                pthread_mutex_destroy (&game.lock);
                end = 1;
                break;
        }
    }

    return NULL;
}

void curses_monitorMaster () {
    if (pthread_create (&tid, NULL, &threadMonitor, NULL) != 0) {
        __mylog (KRED, "Failed to create thread!\n");
        game.state = GAM_END;
        return;
    }

    if (pthread_mutex_init(&game.lock, NULL) != 0) {
        __mylog (KRED, "Failed to init lock!\n");
        game.state = GAM_END;
        return;
    }
}

void curses_destroyUI () {
    endwin ();
}
