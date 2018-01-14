#include "map.c"

void update_map (int x, int y, float dir, int values, int *obstacles, int *angles);

void map_print(int startX, int startY, int endX, int endY);

void map_fix (int x, int y, int dir, int dist, int w, int value);

void add_wall (int startX, int startY, int endX, int endY, int value);

void add_small_arena_walls ();

void add_large_arena_walls ();

void add_my_obstacle(int startX, int startY, int endX, int endY);

int empty_cnt(int y, int x);

int choice_LR(int x, int y, int dir);

void map_average();

void map_average_w(float w);

void print_matrix(int matrix[H_AVG][L_AVG]);

void image_proc(int full,int empty,int boh,int map_proc[H_AVG][L_AVG]);
