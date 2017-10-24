#ifndef GRAPHICS_H
#define GRAPHICS_H

int graphicsInit ();
int graphicsInitWindow (int team1, int team2, int team3, int team4);
void addCoordinate (int team, int x, int y);
void ballAction (int team, int x, int y);
void graphicsDestroyWindow ();
void graphicsQuit ();

#endif
