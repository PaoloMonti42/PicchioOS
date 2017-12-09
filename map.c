
#include <math.h>

#define L 60
#define H 100
#define P 10
#define SURE 0b01
#define HIT 0b11
#define MISS 0b10
#define SURE_HIT 0b1111111111111101
#define SURE_MISS 0b1010101010101001
#define DIST_THRESHOLD 150

#define LEFT -1
#define RIGHT 1
#define VERTICAL 0
#define HORIZONTAL 1
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

uint16_t mat[P+H+P][P+L+P] = {{0}};

void update_map (int x, int y, int dir, int values, int *obstacles, int *angles) {
  int i;
  int r, c;
  int w = 4;
  int t = DIST_THRESHOLD/10;
  float height_ob = 1;
  float * obstaclesF;

  // x+=L/2;
  // y+=20;

  // printf("(x,y)=(%d,%d)\n", x, y);
  obstaclesF = (float *)malloc(sizeof(float)*values);

  for(i=0;i<values;i++){
      obstaclesF[i]= ((float)obstacles[i])/10.0;
  }

  for (i = 0; i < values; i++) {

    float mx = (obstacles[i] == 0 ? t : obstaclesF[i]) * sin((angles[i] * M_PI) / 180.0);
    float my = (obstacles[i] == 0 ? t : obstaclesF[i]) * cos((angles[i] * M_PI) / 180.0);
    float heightx = (obstacles[i] == 0 ? 0 : height_ob) * sin((angles[i] * M_PI) / 180.0);
    float heighty = (obstacles[i] == 0 ? 0 : height_ob) * cos((angles[i] * M_PI) / 180.0);

    float nx = w/2 * sin(((angles[i]+90) * M_PI) / 180.0);
    float ny = w/2 * cos(((angles[i]+90) * M_PI) / 180.0);

    float p1x = x - nx;
    float p1y = y - ny;
    // printf("P1 (%f, %f)\n", p1x, p1y);

    float p2x = x + nx;
    float p2y = y + ny;
    // printf("P2 (%f, %f)\n", p2x, p2y);

    float p3x = p2x + mx;
    float p3y = p2y + my;
    // printf("P3 (%f, %f)\n", p3x, p3y);

    float p4x = p1x + mx;
    float p4y = p1y + my;
    // printf("P4 (%f, %f)\n", p4x, p4y);

    float ob_p1x = p4x;
    float ob_p1y = p4y;
    //printf("P1 (%f, %f)\n", p1x, p1y);

    float ob_p2x = p3x;
    float ob_p2y = p3y;
    //printf("P2 (%f, %f)\n", p2x, p2y);

    float ob_p3x = ob_p2x + heightx;
    float ob_p3y = ob_p2y + heighty;
    //printf("P3 (%f, %f)\n", p3x, p3y);

    float ob_p4x = ob_p1x + heightx;
    float ob_p4y = ob_p1y + heighty;

    // printf("coordinates: (%f, %f), (%f, %f), (%f, %f), (%f, %f)\n", ob_p1x, ob_p1y, ob_p2x, ob_p2y, ob_p3x, ob_p3y, ob_p4x, ob_p4y);


    int boundDX = x+t>P+L-1?P+L-1:x+t;
    int boundSX = x-t<P?P:x-t;

    int boundUP = y+t+1>P+H-1?P+H-1:y+t+1;
    int boundDW = y-t<P?P:y-t;

    // printf("dx: %d, sx: %d, up: %d, dw: %d\n",boundDX, boundSX, boundUP, boundDW);

    for (r = boundUP; r > boundDW; r--) {
      for (c = boundSX; c < boundDX; c++) {
        if ( (p2y-p1y)*c - (p2x-p1x)*r + p2x*p1y - p2y*p1x <= 0 &&
             (p3y-p2y)*c - (p3x-p2x)*r + p3x*p2y - p3y*p2x <= 0 &&
             (p4y-p3y)*c - (p4x-p3x)*r + p4x*p3y - p4y*p3x <= 0 &&
             (p1y-p4y)*c - (p1x-p4x)*r + p1x*p4y - p1y*p4x <= 0 ) {
          if ((mat[r][c] & 0b11) != SURE) {
            mat[r][c] = (mat[r][c] << 2) + MISS;
            // printf("0 ");
          }
        } else if (!(heightx==0 && heighty==0) &&(ob_p2y-ob_p1y)*c - (ob_p2x-ob_p1x)*r + ob_p2x*ob_p1y - ob_p2y*ob_p1x <= 0 &&
             (ob_p3y-ob_p2y)*c - (ob_p3x-ob_p2x)*r + ob_p3x*ob_p2y - ob_p3y*ob_p2x <= 0 &&
             (ob_p4y-ob_p3y)*c - (ob_p4x-ob_p3x)*r + ob_p4x*ob_p3y - ob_p4y*ob_p3x <= 0 &&
             (ob_p1y-ob_p4y)*c - (ob_p1x-ob_p4x)*r + ob_p1x*ob_p4y - ob_p1y*ob_p4x <= 0 ) {
            if ((mat[r][c] & 0b11) != SURE) {
              mat[r][c] = (mat[r][c] << 2) + HIT;
              // printf("1 ");
            }
       }  else {
          // fuori dal campo visivo, rimane unknown
          // printf(". ");
        }
      }
      // printf("\n");
    }
    // printf("------------------------------------------------------\n");
  }

}

void map_print(int startX, int startY, int endX, int endY) {
  int r, c, i;

  FILE * fp = fopen("map.txt", "w+");
  for (r = endY-1; r >= startY; r--) {
    for (c = startX; c < endX; c++) {
      uint16_t tmp = mat[r][c];
      for (i = 7; i >= 0; i--) {
        switch ((tmp >> i*2) & 0b11)  {
          case (0):
            fprintf(fp, "?");
            break;
          case (1):
            fprintf(fp, "X");
            break;
          case (2):
            fprintf(fp, "_");
            break;
          case (3):
            fprintf(fp, "@");
            break;
        }
      }
      fprintf(fp, " ");
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void map_fix (int x, int y, int dir, int dist, int value) {
  int r, c;
  int w = 17;

  float mx = dist * sin((dir * M_PI) / 180.0);
  float my = dist * cos((dir * M_PI) / 180.0);

  float nx = w/2 * sin(((dir+90) * M_PI) / 180.0);
  float ny = w/2 * cos(((dir+90) * M_PI) / 180.0);

  float p1x = x - nx;
  float p1y = y - ny;
  // printf("P1 (%f, %f)\n", p1x, p1y);

  float p2x = x + nx;
  float p2y = y + ny;
  // printf("P2 (%f, %f)\n", p2x, p2y);

  float p3x = p2x + mx;
  float p3y = p2y + my;
  // printf("P3 (%f, %f)\n", p3x, p3y);

  float p4x = p1x + mx;
  float p4y = p1y + my;
  // printf("P4 (%f, %f)\n", p4x, p4y);

  for (r = P+H+P; r > 0; r--) {
    for (c = 0; c < P+L+P; c++) {
      if ( (p2y-p1y)*c - (p2x-p1x)*r + p2x*p1y - p2y*p1x <= 0 &&
           (p3y-p2y)*c - (p3x-p2x)*r + p3x*p2y - p3y*p2x <= 0 &&
           (p4y-p3y)*c - (p4x-p3x)*r + p4x*p3y - p4y*p3x <= 0 &&
           (p1y-p4y)*c - (p1x-p4x)*r + p1x*p4y - p1y*p4x <= 0 ) {
       if ((mat[r][c] & 0b11) != SURE) {
         mat[r][c] = value;
       }
      }
    }
  }
}

void add_wall (int startX, int startY, int endX, int endY, int value)
{
  int r, c;
  for (r = endY-1; r >= startY; r--) {
    for (c = startX; c < endX; c++) {
      mat[r][c] = value;
    }
  }
}

void add_my_obstacle(int startX, int startY, int endX, int endY)
{
  //add_my_obstacle(my_pos.x-SIDEX_OBSTACLE/2, my_pos.y-TAIL_CORRECTION-SIDEY_OBSTACLE, my_pos.x+SIDEX_OBSTACLE/2, my_pos.y-TAIL_CORRECTION);
  add_wall(startX, startY, endX, endY, SURE_HIT);
}

int empty_cnt(int y, int x){
  int a=0, k;
  if((mat[y][x] & 0b11)==0b01){
    //printf("In y: %d and x: %d found: %d zeros.\n", y, x, a);
		return 0;
	}
  for(k=0; k<8; k++){
	   if(((mat[y][x] >> (2*k)) & 0b11)==0)
		   a++;
  }
	//printf("In y: %d and x: %d found: %d zeros.\n", y, x, a);
  return a;
}


int choice_LR(int x, int y, int dir){
  int i,j;
  int check1=0, check2=0;
	int orientation, direction;

	if(dir <= 45 && dir > -45){
		orientation=VERTICAL;
		direction=NORTH;
	}else if(dir <= 135 && dir > 45){
		orientation=HORIZONTAL;
		direction=EAST;
	}else if(dir <= -135 || dir > 135){
		orientation=VERTICAL;
		direction=SOUTH;
	}else{
		orientation=HORIZONTAL;
		direction=WEST;
	}

	//VERTICAL SPAN
	if(orientation==VERTICAL){
 		for(i=0; i<H; i++){
  	  for(j=0; j<x; j++){
				// printf("y: %d, x: %d.\n", i, j);
	      check1+=empty_cnt(i,j);
 		  }
		  for(j=x; j<L; j++){
        // printf("y: %d, x: %d.\n", i, j);
  			check2+=empty_cnt(i,j);
      }
    }
	//HORIZONTAL SPAN
	}else{
		for(j=0; j<L; j++){
			// printf("check1:\n");
      for(i=0; i<y; i++){
        check1+=empty_cnt(i,j);
      }
			// printf("check2:\n");
      for(i=y; i<H; i++){
        check2+=empty_cnt(i,j);
      }
    }
	}

	// printf("Check1: %d, check2: %d, direction: %d, orientation: %d.\n", check1, check2, direction, orientation);

  if(check1>check2){
		if(direction==NORTH || direction==WEST){
			return LEFT;
		}else if(direction==SOUTH || direction==EAST){
			return RIGHT;
		}
  }else{
    if(direction==NORTH || direction==WEST){
			return RIGHT;
		}else if(direction==SOUTH || direction==EAST){
			return LEFT;
		}
  }
  return 0;
}
