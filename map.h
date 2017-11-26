#include "map.c"

void update_map (int x, int y, int dir, int values, int *obstacles, int *angles);

void map_print(int startX, int startY, int endX, int endY);

void map_fix (int x, int y, int dir, int dist, int value);
