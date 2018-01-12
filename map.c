
#include <math.h>
#include <stdint.h>

#define L 120
#define H 200
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

#define MAP_SQUARE 5

uint16_t mat[P+H+P][P+L+P] = {{0}};

void update_map (int x, int y, float dir, int values, int *obstacles, int *angles) {
  int i;
  int r, c;
  int w = 4;
  int f = 8;
  int t = DIST_THRESHOLD/10;
  float height_ob = 15;
  float * obstaclesF;

  for(i=0;i<values;i++){
      angles[i] = angles[i]+dir;
      if (angles[i] > 180) angles[i] -= 360;
      if (angles[i] < - 180) angles[i] += 360;
  }

  // rotation point of eyes
  float fx = x + f * sin((angles[values/2] * M_PI) / 180.0);
  float fy = y + f * cos((angles[values/2] * M_PI) / 180.0);

  // printf("%d %d %f %f %f %d %d %d\n", x, y, dir, fx, fy, values/2, angles[values/2], obstacles[values/2]);

  // printf("(x,y)=(%d,%d)\n", x, y);
  obstaclesF = (float *)malloc(sizeof(float)*values);

  for(i=0;i<values;i++){
      obstaclesF[i]= ((float)obstacles[i])/10.0;
  }

  for (i = 0; i < values; i++) {


    // exact point of eyes
    float ex = fx + 2 * sin((angles[i] * M_PI) / 180.0);
    float ey = fy + 2 * cos((angles[i] * M_PI) / 180.0);
    // printf("angles[%d] = %d, fx = %f fy = %f, ex = %f, ey = %f\n", i, angles[i], fx-P, fy-P, ex-P, ey-P);

    float mx = (obstacles[i] == 0 ? t : obstaclesF[i]) * sin((angles[i] * M_PI) / 180.0);
    float my = (obstacles[i] == 0 ? t : obstaclesF[i]) * cos((angles[i] * M_PI) / 180.0);
    float heightx = (obstacles[i] == 0 ? 0 : height_ob) * sin((angles[i] * M_PI) / 180.0);
    float heighty = (obstacles[i] == 0 ? 0 : height_ob) * cos((angles[i] * M_PI) / 180.0);

    float nx = w/2 * sin(((angles[i]+90) * M_PI) / 180.0);
    float ny = w/2 * cos(((angles[i]+90) * M_PI) / 180.0);

    float p1x = ex - nx;
    float p1y = ey - ny;
    // printf("P1 (%f, %f)\n", p1x, p1y);

    float p2x = ex + nx;
    float p2y = ey + ny;
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


    int boundDX = ex+t>P+L-1?P+L-1:ex+t;
    int boundSX = ex-t<P?P:ex-t;

    int boundUP = ey+t+1>P+H-1?P+H-1:ey+t+1;
    int boundDW = ey-t<P?P:ey-t;

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

void map_fix (int x, int y, int dir, int dist, int w, int value) {
  //printf("%d %d %d %d %d\n", x, y, dir, dist, value);
  int r, c;

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
        if ((mat[r][c] & 0b11) != SURE || value == SURE_HIT) {
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

void add_small_arena_walls () {
  add_wall(0, 0, P+L+P, P, SURE_HIT);							// bottom
  add_wall(0, 0, P, P+H+P, SURE_HIT);							// left
  add_wall(0, P+H, P+L+P, P+H+P, SURE_HIT);				// top
  add_wall(P+L, 0, P+L+P, P+H+P, SURE_HIT);				// right
}

void add_large_arena_walls () {
  add_wall(0, 0, P+L+P, P, SURE_HIT);							// bottom


  // TAKE PADDING INTO ACCOUNT!!!
  // add_wall (int startX, int startY, int endX, int endY, SURE_HIT);
  // add_wall (int startX, int startY, int endX, int endY, SURE_HIT);
  // add_wall (int startX, int startY, int endX, int endY, SURE_HIT);
  // add_wall (int startX, int startY, int endX, int endY, SURE_HIT);
  // add_wall (int startX, int startY, int endX, int endY, SURE_HIT);
  // add_wall (int startX, int startY, int endX, int endY, SURE_HIT);
}

void add_my_obstacle(int startX, int startY, int endX, int endY)
{
  //add_my_obstacle(my_pos.x-SIDEX_OBSTACLE/2, my_pos.y-TAIL-SIDEY_OBSTACLE, my_pos.x+SIDEX_OBSTACLE/2, my_pos.y-TAIL);
  //printf("startx:%d, starty:%d, endx:%d, endy:%d\n", startX, startY, endX, endY);
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

void map_average(){
  int row_ext, col_ext, row_int, col_int, k;
  int average[MAP_SQUARE][MAP_SQUARE]={{0}};
  int average_square=0;
  int flag=0;

  for (row_ext = P+H+P; row_ext > 0; row_ext-=5) {
    //printf("row_ext=%d\n", row_ext);
    for (col_ext = 0; col_ext < P+L+P; col_ext+=5) {
      //printf("col_ext=%d\n", col_ext);
      for(row_int=row_ext-MAP_SQUARE; row_int<row_ext; row_int++){
        //printf("row_int=%d\n", row_int);
        for(col_int=col_ext; col_int < col_ext+MAP_SQUARE; col_int++){
          //printf("col_int=%d\n", col_int);
            if((mat[row_int][col_int] & 0b11)==0b01){
              if((((mat[row_int][col_int] >> 2) & 0b11))==HIT){
                average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=8;
                flag=1;
              } else {
                average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=-8;
                flag=1;
              }
            } else {
              for(k=0; k<8; k++){
                //printf("a\n");
                if((((mat[row_int][col_int] >> (2*k)) & 0b11))==HIT){
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]+=1;
                  flag=1;
                  //printf("FULL HERE\n");
                } else if((((mat[row_int][col_int] >> (2*k)) & 0b11))==MISS){
                  //average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]-=1;
                  //flag=1;
                }
              }
            }
                //printf("average=%d\n", average[row_int][col_int]);
                if(flag==0){
                  average_square=0;
                }
                else if(average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]>=0){
                  average_square+=1;
                  } else {
                    average_square-=1;
                  }
                average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]=0;
                flag=0;
            }
        }
      //printf("average of the square is: %d\n", average_square);
      if(average_square>0 && average_square<=25){
        //printf("average_square=%d\n", average_square);
        printf("@ ");
      } else if (average_square<0) {
          printf("_ ");
      } else {
        printf("? ");
      }

      average_square=0;
      }
        printf("\n");
    }

  //printf("finish\n");
  return;
}


void map_average_w(float w){
  int row_ext, col_ext, row_int, col_int, k;
  float average[MAP_SQUARE][MAP_SQUARE]={{0}};
  int average_square=0;
  int flag=0;

  for (row_ext = P+H+P; row_ext > 0; row_ext-=5) {
    //printf("row_ext=%d\n", row_ext);
    for (col_ext = 0; col_ext < P+L+P; col_ext+=5) {
      //printf("col_ext=%d\n", col_ext);
      for(row_int=row_ext-MAP_SQUARE; row_int<row_ext; row_int++){
        //printf("row_int=%d\n", row_int);
        for(col_int=col_ext; col_int < col_ext+MAP_SQUARE; col_int++){
          //printf("col_int=%d\n", col_int);
            if((mat[row_int][col_int] & 0b11)==0b01){
              if((((mat[row_int][col_int] >> 2) & 0b11))==HIT){
                average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=8;
                flag=1;
              } else {
                average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=-8;
                flag=1;
              }
            } else {
              for(k=0; k<8; k++){
                //printf("a\n");
                if((((mat[row_int][col_int] >> (2*k)) & 0b11))==HIT){
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]+=1;
                  flag=1;
                  //printf("FULL HERE\n");
                } else if((((mat[row_int][col_int] >> (2*k)) & 0b11))==MISS){
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]-=w;
                  flag=1;
                }
              }
            }
                //printf("average=%d\n", average[row_int][col_int]);
                if(flag==0){
                  average_square=0;
                }
                else if(average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]>=0){
                  average_square+=1;
                  } else {
                    average_square-=1;
                  }
                average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]=0;
                flag=0;
            }
        }
      //printf("average of the square is: %d\n", average_square);
      if(average_square>0 && average_square<=25){
        //printf("average_square=%d\n", average_square);
        printf("@ ");
      } else if (average_square<0) {
          printf("_ ");
      } else {
        printf("? ");
      }

      average_square=0;
      }
        printf("\n");
    }

  //printf("finish\n");
  return;
}
