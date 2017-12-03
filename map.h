#include "map.c"

int isempty(int i, int j);

int choice_UD();

int choidce_LR();

void update_map (int x, int y, int dir, int values, int *obstacles, int *angles);

void map_print(int startX, int startY, int endX, int endY);
