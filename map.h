#include "map.c"

void update_map (int x, int y, int dir, int values, int *obstacles, int *angles);

void map_print(int startX, int startY, int endX, int endY);

void map_fix (int x, int y, int dir, int dist, int value);

int empty_cnt(int y, int x);

int choice_LR(int x, int y, int dir);

void add_wall (int startX, int startY, int endX, int endY, int value);

void add_my_obstacle(int startX, int startY, int endX, int endY);
