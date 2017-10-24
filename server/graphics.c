#ifndef GRAPHICS

int graphicsInit () { return 0; }
int graphicsInitWindow (int team1, int team2, int team3, int team4) { return 0; }
void addCoordinate (int team, int x, int y) {}
void ballAction (int team, int x, int y) {}
void graphicsDestroyWindow () {}
void graphicsQuit () {}

#else

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <pthread.h>
#include <string.h>
#include <math.h>

#include "graphics.h"
#include "ui.h"

#define log __mylog

#define CMTOPXRATIO         1.5
#define PXBORDER            20
#define COORD_PER_PLAYER    300
#define PX(v)               ((int) (v*CMTOPXRATIO))

#define OBSTACLE_WIDTH      15
#define START_AREA_WIDTH    40
#define START_AREA_MARGIN   10

#define DEG2RAD             0.01745

#define DELAY_FRAME_RENDER  1000

#define NBCOLORS        5

struct {
   float r;
   float g;
   float b;
} playerColors_graphics [] = {
    {0.3059, 0.6039, 0.0235},
    {0.7686, 0.6275, 0},
    {0.2039, 0.3961, 0.6431},
    {0.4588, 0.3137, 0.4824},
    {0.1373, 0.5961, 0.6196}
};

#define COL_G(i) playerColors_graphics[i % NBCOLORS]

struct coordinate {
    int x;
    int y;
};

/* Init SDL */
int graphicsInit () {
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        SDL_Quit ();

        return -1;
    }

    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    return 0;
}

SDL_Window *window = NULL;
SDL_GLContext openGLContext;
struct coordinate * coordinates = NULL;
struct coordinate * newCoordinates[4];
struct coordinate ballCoordinates[4];
unsigned char teamIndex[15];
int nbPlayers;
pthread_mutex_t lock;
pthread_t tid;

void drawCurrentPos (int x, int y) {
    int i;
    glBegin (GL_LINE_LOOP);
    for (i=0; i < 360; i+=10) {
        float degInRad = i*DEG2RAD;
        glVertex2f (cos (degInRad)*5+x, sin (degInRad)*5+y);
    }
    glEnd ();
}

void drawBalls () {
    int i;

    pthread_mutex_lock (&lock);

    for (i=0; i<4; i++) {
        if (ballCoordinates[i].y >= 0) {
            glColor3f (.2,.2,.2);
            glBegin(GL_QUADS);
                glVertex2f(ballCoordinates[i].x-3, ballCoordinates[i].y-3);
                glVertex2f(ballCoordinates[i].x-3, ballCoordinates[i].y+3);
                glVertex2f(ballCoordinates[i].x+3, ballCoordinates[i].y+3);
                glVertex2f(ballCoordinates[i].x+3, ballCoordinates[i].y-3);
            glEnd();
        }
    }

    pthread_mutex_unlock (&lock);
}

void drawPath () {
    int team;

    pthread_mutex_lock (&lock);

    for (team=0; team<15; team++) {
        if (teamIndex[team] != 255) {
            struct coordinate *coord, *newCoord;
            coord = &coordinates[COORD_PER_PLAYER*teamIndex[team]];
            newCoord = newCoordinates[teamIndex[team]];

            glColor3f (COL_G(team).r, COL_G(team).g, COL_G(team).b);

            if (newCoord > coord+1) {
                struct coordinate *iter;
                glBegin (GL_LINE_STRIP);
                for (iter = coord; iter < newCoord; iter++) {
                    glVertex2f (iter->x, iter->y);
                }
                glEnd ();
            }

            if (newCoord != coord) {
                drawCurrentPos ((newCoord-1)->x, (newCoord-1)->y);
            }
        }
    }

    pthread_mutex_unlock (&lock);
}

void drawStartDestArea (int x, int y) {
    glBegin (GL_LINE_STRIP);
        glVertex2f (x, y+10);
        glVertex2f (x, y);
        glVertex2f (x+10, y);
    glEnd ();

    glBegin (GL_LINE_STRIP);
        glVertex2f (x+START_AREA_WIDTH-10, y);
        glVertex2f (x+START_AREA_WIDTH, y);
        glVertex2f (x+START_AREA_WIDTH, y + 10);
    glEnd ();

    glBegin (GL_LINE_STRIP);
        glVertex2f (x+START_AREA_WIDTH, y+START_AREA_WIDTH-10);
        glVertex2f (x+START_AREA_WIDTH, y+START_AREA_WIDTH);
        glVertex2f (x+START_AREA_WIDTH-10, y+START_AREA_WIDTH);
    glEnd ();

    glBegin (GL_LINE_STRIP);
        glVertex2f (x+10, y+START_AREA_WIDTH);
        glVertex2f (x, y+START_AREA_WIDTH);
        glVertex2f (x, y+START_AREA_WIDTH-10);
    glEnd ();
}

void drawArena () {
    /* Draw arena */
    glColor3f (1,1,1);
    if (nbPlayers == 2) {
        glBegin(GL_QUADS);
            glVertex2f(0, 200);
            glVertex2f(0, 0);
            glVertex2f(120, 0);
            glVertex2f(120, 200);
        glEnd();

        drawStartDestArea (START_AREA_MARGIN, START_AREA_MARGIN);
        drawStartDestArea (120-START_AREA_MARGIN-START_AREA_WIDTH, START_AREA_MARGIN);
        drawStartDestArea (120-START_AREA_MARGIN-START_AREA_WIDTH, 200-START_AREA_MARGIN-START_AREA_WIDTH);
        drawStartDestArea (START_AREA_MARGIN, 200-START_AREA_MARGIN-START_AREA_WIDTH);
    } else {
        glBegin(GL_QUADS);
            glVertex2f(-120, 400);
            glVertex2f(-120, 0);
            glVertex2f(120, 0);
            glVertex2f(120, 400);
        glEnd();

        glColor3f (0,0,0);
        glBegin (GL_LINES);
            glVertex2f (0, 0);
            glVertex2f (0, 400);
        glEnd();

        glColor3f (0,0,0.8);
        glBegin (GL_QUADS);
            glVertex2f (-60, 150);
            glVertex2f (60, 150);
            glVertex2f (60, 150+OBSTACLE_WIDTH);
            glVertex2f (-60, 150+OBSTACLE_WIDTH);
        glEnd ();

        glBegin (GL_QUADS);
            glVertex2f (-120, 250-OBSTACLE_WIDTH);
            glVertex2f (-60, 250-OBSTACLE_WIDTH);
            glVertex2f (-60, 250);
            glVertex2f (-120, 250);
        glEnd ();

        glBegin (GL_QUADS);
            glVertex2f (120, 250-OBSTACLE_WIDTH);
            glVertex2f (60, 250-OBSTACLE_WIDTH);
            glVertex2f (60, 250);
            glVertex2f (120, 250);
        glEnd ();

        drawStartDestArea (-120+START_AREA_MARGIN, START_AREA_MARGIN);
        drawStartDestArea (-START_AREA_MARGIN-START_AREA_WIDTH, START_AREA_MARGIN);
        drawStartDestArea (-START_AREA_MARGIN-START_AREA_WIDTH, 400-START_AREA_MARGIN-START_AREA_WIDTH);
        drawStartDestArea (-120+START_AREA_MARGIN, 400-START_AREA_MARGIN-START_AREA_WIDTH);
        drawStartDestArea (START_AREA_MARGIN, START_AREA_MARGIN);
        drawStartDestArea (120-START_AREA_MARGIN-START_AREA_WIDTH, START_AREA_MARGIN);
        drawStartDestArea (120-START_AREA_MARGIN-START_AREA_WIDTH, 400-START_AREA_MARGIN-START_AREA_WIDTH);
        drawStartDestArea (START_AREA_MARGIN, 400-START_AREA_MARGIN-START_AREA_WIDTH);
    }
}

void glLoop () {
    while (window) {
        SDL_Event event;

        if (!window) break;

        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    graphicsDestroyWindow ();
                    return;
            }
        }


        /* Clear buffer */
        glClear (GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        drawArena ();

        drawBalls ();

        drawPath ();

        SDL_GL_SwapWindow (window);

        SDL_Delay (DELAY_FRAME_RENDER);
    }
}

void * graphicsInitWindowAux (void * __dummy) {
    window = SDL_CreateWindow (
            "OS Contest",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            PX(60*nbPlayers) + 2*PXBORDER,
            PX(100*nbPlayers) + 2*PXBORDER,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
            );

    if (!window) {
        free (coordinates);
        coordinates = NULL;
        return (void *) -1;
    }

    if (pthread_mutex_init(&lock, NULL) != 0) {
        SDL_DestroyWindow (window);
        free (coordinates);
        coordinates = NULL;
        window = NULL;

        return (void *) -1;
    }

    if (!(openGLContext = SDL_GL_CreateContext (window))) {
        SDL_DestroyWindow (window);
        pthread_mutex_destroy (&lock);
        free (coordinates);
        coordinates = NULL;
        window = NULL;

        return (void *) -1;
    }

    glewExperimental = GL_TRUE;
    if (glewInit () != GLEW_OK) {
        SDL_GL_DeleteContext (openGLContext);
        SDL_DestroyWindow (window);
        pthread_mutex_destroy (&lock);
        free (coordinates);
        coordinates = NULL;
        window = NULL;

        return (void *) -1;
    }


    if (nbPlayers == 4) {
        glViewport(PXBORDER, PXBORDER, PX(240), PX(400));
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-120, 120, 0, 400);
    } else {
        glViewport(PXBORDER, PXBORDER, PX(120), PX(200));
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, 120, 0, 200);
    }


    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor (0, 0, 0, 1);

    glLoop ();

    return NULL;
}

/* Init SDL Window */
int graphicsInitWindow (int team1, int team2, int team3, int team4) {
    int teams[4];
    int i;

    nbPlayers = (team3 >= 0) ? 4 : 2;
    teams[0] = team1;
    teams[1] = team2;
    teams[2] = team3;
    teams[3] = team4;

    coordinates = (struct coordinate *) malloc (COORD_PER_PLAYER*nbPlayers*sizeof(struct coordinate));
    for (i=0; i < 4; i++) {
        newCoordinates[i] = &coordinates[COORD_PER_PLAYER*i];
    }

    memset (teamIndex, 255, sizeof(teamIndex));
    for (i=0; i<4; i++) {
        if (((int *) teams)[i] >= 0) {
            teamIndex[((int *) teams)[i]] = i;
        }
    }
    for (i=0; i<4; i++) {
        ballCoordinates[i].y = -1;
    }

    if (pthread_create (&tid, NULL, &graphicsInitWindowAux, teams) != 0) {
        return -1;
    }

    return 0;
}

void ballAction (int team, int x, int y) {
    if (!window)
        return;
    if (teamIndex[team] == 255)
        return;

    if (ballCoordinates[teamIndex[team]].y < 0) {
        if (x > 120 || x < -120 || (x < 0 && nbPlayers == 2) || y < 0 || y > 100 * nbPlayers)
            return;

        pthread_mutex_lock (&lock);
        ballCoordinates[teamIndex[team]].x = x;
        ballCoordinates[teamIndex[team]].y = y;
        pthread_mutex_unlock (&lock);
    } else {
        pthread_mutex_lock (&lock);
        ballCoordinates[teamIndex[team]].y = -1;
        pthread_mutex_unlock (&lock);
    }
}

void addCoordinate (int team, int x, int y) {
    if (!coordinates)
        return;
    if (x > 120 || x < -120 || (x < 0 && nbPlayers == 2) || y < 0 || y > 100 * nbPlayers)
        return;
    if (teamIndex[team] == 255)
        return;

    pthread_mutex_lock (&lock);

    newCoordinates[teamIndex[team]]->x = x;
    newCoordinates[teamIndex[team]]->y = y;
    newCoordinates[teamIndex[team]]++;

    pthread_mutex_unlock (&lock);
}

/* Destroy SDL Window */
void graphicsDestroyWindow () {
    if (window) {
        pthread_mutex_lock (&lock);
        free (coordinates);
        coordinates = NULL;
        SDL_GL_DeleteContext (openGLContext);
        pthread_mutex_destroy (&lock);
        SDL_DestroyWindow (window);
    }
    window = NULL;
}

/* Quit SDL */
void graphicsQuit () {
    SDL_Quit ();
}

#endif
