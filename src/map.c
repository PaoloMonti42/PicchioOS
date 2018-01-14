
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define L 75
#define H 110
#define P 10
#define L_AVG (P+L+P)/5
#define H_AVG (P+H+P)/5
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
int map_copy[H_AVG][L_AVG];

/*
 * Function: update_map
 * --------------------
 * Saves in the map the obstacles identified by the robot
 * --------------------
 * x: x coordinate of the robot
 * y: y coordintae of the robot
 * dir: direction of the robot in the range (-180:180) where 0 means front -90 left 90 right
 * values: number of readings performed by the robot with the scan function
 * obstacles: array of the distances of the obstacles identified by the robot with 0 that means no obstacle. sorted with the same rule used for angles
 * angles: array of the relative angles (-180:180) in which the robot has perfmormed a scan. in position 0 there is the leftmost scan
 * --------------------
 * Made by Paolo & Martina & Luca
 */
void update_map (int x, int y, float dir, int values, int *obstacles, int *angles) {
  int i;

  int r, c;
  int width = 4;
  int face = 8;
  int th = DIST_THRESHOLD/10;
  float obs_depth = 15;
  float * obstaclesF;

  //Apply the array of relative angles to the current position
  for(i=0;i<values;i++){
      angles[i] = angles[i]+dir;
      if (angles[i] > 180) angles[i] -= 360;
      if (angles[i] < - 180) angles[i] += 360;
  }

  // Compute the rotation point of eyes
  float fx = x + face * sin((angles[values/2] * M_PI) / 180.0);
  float fy = y + face * cos((angles[values/2] * M_PI) / 180.0);

  obstaclesF = (float *)malloc(sizeof(float)*values);

  for(i=0;i<values;i++){
      obstaclesF[i]= ((float)obstacles[i])/10.0;
  }

  for (i = 0; i < values; i++) {
    // Compute the exact point of eyes
    float ex = fx + 2 * sin((angles[i] * M_PI) / 180.0);
    float ey = fy + 2 * cos((angles[i] * M_PI) / 180.0);

    //Geometry ahead

    //Find the slope of the line of the scan
    float mx = (obstacles[i] == 0 ? th : obstaclesF[i]) * sin((angles[i] * M_PI) / 180.0);
    float my = (obstacles[i] == 0 ? th : obstaclesF[i]) * cos((angles[i] * M_PI) / 180.0);
    float heightx = (obstacles[i] == 0 ? 0 : obs_depth) * sin((angles[i] * M_PI) / 180.0);
    float heighty = (obstacles[i] == 0 ? 0 : obs_depth) * cos((angles[i] * M_PI) / 180.0);

    //Find the slope of the other line of the scan
    float nx = width/2 * sin(((angles[i]+90) * M_PI) / 180.0);
    float ny = width/2 * cos(((angles[i]+90) * M_PI) / 180.0);

    //Coordinates of the scanned area
    float p1x = ex - nx;
    float p1y = ey - ny;

    float p2x = ex + nx;
    float p2y = ey + ny;

    float p3x = p2x + mx;
    float p3y = p2y + my;

    float p4x = p1x + mx;
    float p4y = p1y + my;

    float ob_p1x = p4x;
    float ob_p1y = p4y;

    float ob_p2x = p3x;
    float ob_p2y = p3y;

    float ob_p3x = ob_p2x + heightx;
    float ob_p3y = ob_p2y + heighty;

    float ob_p4x = ob_p1x + heightx;
    float ob_p4y = ob_p1y + heighty;

    //Compute bound to avoid writing OOB
    int boundDX = ex+th>P+L-1?P+L-1:ex+th;
    int boundSX = ex-th<P?P:ex-th;

    int boundUP = ey+th+1>P+H-1?P+H-1:ey+th+1;
    int boundDW = ey-th<P?P:ey-th;

    for (r = boundUP; r > boundDW; r--) {
      for (c = boundSX; c <= boundDX; c++) {
        //Compute if a given point of the map is inside the rectagle delimited by the four coordinates
        if ( (p2y-p1y)*c - (p2x-p1x)*r + p2x*p1y - p2y*p1x <= 0 &&
             (p3y-p2y)*c - (p3x-p2x)*r + p3x*p2y - p3y*p2x <= 0 &&
             (p4y-p3y)*c - (p4x-p3x)*r + p4x*p3y - p4y*p3x <= 0 &&
             (p1y-p4y)*c - (p1x-p4x)*r + p1x*p4y - p1y*p4x <= 0 ) {
          if ((mat[r][c] & 0b11) != SURE) {
            //Shift the current cell to add a measurement
            mat[r][c] = (mat[r][c] << 2) + MISS;
          }
        } else if (!(heightx==0 && heighty==0) &&(ob_p2y-ob_p1y)*c - (ob_p2x-ob_p1x)*r + ob_p2x*ob_p1y - ob_p2y*ob_p1x <= 0 &&
             (ob_p3y-ob_p2y)*c - (ob_p3x-ob_p2x)*r + ob_p3x*ob_p2y - ob_p3y*ob_p2x <= 0 &&
             (ob_p4y-ob_p3y)*c - (ob_p4x-ob_p3x)*r + ob_p4x*ob_p3y - ob_p4y*ob_p3x <= 0 &&
             (ob_p1y-ob_p4y)*c - (ob_p1x-ob_p4x)*r + ob_p1x*ob_p4y - ob_p1y*ob_p4x <= 0 ) {
            if ((mat[r][c] & 0b11) != SURE) {
              mat[r][c] = (mat[r][c] << 2) + HIT;
            }
       }  else {
        }
      }
    }
  }
}


/*
 * Function: map_print
 * --------------------
 * Saves the map in logs/map.txt file
 * --------------------
 * startX: x coordinate of starting point of the map to be saved in the file
 * startY: y coordinate of starting point of the map to be saved in the file
 * endX: x coordinate of ending point of the map to be saved in the file
 * endY: y coordinate of ending point of the map to be saved in the file
 * --------------------
 * Made by Paolo
 */
void map_print(int startX, int startY, int endX, int endY) {
  int r, c, i;

  FILE * fp = fopen("logs/map.txt", "w+");
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

/*
 * Function: map_fix
 * --------------------
 * Save the path followed by the robot in the map
 * --------------------
 * x: x coordinate of starting point of the robot
 * y: y coordinate of starting point of the robot
 * dir: direction of the robot in the range (-180:180) where 0 means front -90 left 90 right
 * dist: distance traveld by the robot
 * w: width of the robot
 * value: value to be written in the map in the path travelled by the robot
 * --------------------
 * Made by Paolo
 */
void map_fix (int x, int y, int dir, int dist, int w, int value) {
  int r, c;

  //Similar geometry comutations to update_map
  //Aim is always to find the 4 coordinates of the area to cover
  float mx = dist * sin((dir * M_PI) / 180.0);
  float my = dist * cos((dir * M_PI) / 180.0);

  float nx = w/2 * sin(((dir+90) * M_PI) / 180.0);
  float ny = w/2 * cos(((dir+90) * M_PI) / 180.0);

  float p1x = x - nx;
  float p1y = y - ny;

  float p2x = x + nx;
  float p2y = y + ny;

  float p3x = p2x + mx;
  float p3y = p2y + my;

  float p4x = p1x + mx;
  float p4y = p1y + my;

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

/*
 * Function: add_wall
 * --------------------
 * Adds a wall in the map
 * --------------------
 * startX: x coordinate of the starting point of the wall
 * startY: y coordinate of the starting point of the wall
 * endX: x coordinate of the ending point of the wall
 * endY: y coordinate of the ending point of the wall
 * value: value to be written in the map for the wall
 * --------------------
 * Made by Paolo & Luca
 */
void add_wall (int startX, int startY, int endX, int endY, int value)
{
  int r, c;
  for (r = endY-1; r >= startY; r--) {
    for (c = startX; c < endX; c++) {
      mat[r][c] = value;
    }
  }
}

/*
 * Function: add_small_arena_walls
 * --------------------
 * Add the walls of the small arena in the map
 * --------------------
 * Made by Paolo
 */
void add_small_arena_walls () {
  add_wall(0, 0, P+L+P, P, SURE_HIT);							// bottom
  add_wall(0, 0, P, P+H+P, SURE_HIT);							// left
  add_wall(0, P+H, P+L+P, P+H+P, SURE_HIT);				// top
  add_wall(P+L, 0, P+L+P, P+H+P, SURE_HIT);				// right
}

/*
 * Function: add_large_arena_walls
 * --------------------
 * Add the walls of the large arena in the map
 * --------------------
 * Made by Paolo
 */
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

/*
 * Function: add_my_obstacle
 * --------------------
 * Wrapper of the add_wall function used to mark the obstacle relased by the robot in the map
 * --------------------
 * startX: x coordinate of the starting point of the wall
 * startY: y coordinate of the starting point of the wall
 * endX: x coordinate of the ending point of the wall
 * endY: y coordinate of the ending point of the wall
 * --------------------
 * Made by Martina
 */
void add_my_obstacle(int startX, int startY, int endX, int endY)
{
  add_wall(startX, startY, endX, endY, SURE_HIT);
}

/*
 * Function: empty_cnt
 * —------------------
 * Counts the number of not yet defined readings for a point in the map
 * —------------------
 * x: x coordinate of the point in the map
 * y: y coordinate of the point in the map
 * —------------------
 * return: the number of undefined reading for that point or 0 if it is a SURE_HIT or SURE_MISS
 * —------------------
 * Made by Valerio
 */
int empty_cnt(int y, int x){
  int a=0, k;
  if((mat[y][x] & 0b11)==0b01){
		return 0;
	}
  for(k=0; k<8; k++){
	   if(((mat[y][x] >> (2*k)) & 0b11)==0)
		   a++;
  }
  return a;
}

/*
 * Function: choice_LR
 * —------------------
 * Chooses the next turn direction of the robot basd on which area of the map is the most unexplored
 * —------------------
 * x: x coordinate of the robot in the map
 * y: y coordinate of the robot in the map
 * dir: direction of the robot
 * —------------------
 * return: the choosen direction LEFT,RIGHT or 0 that means LEFT in case of even result
 * —------------------
 * Made by Martina & Valerio
 */
int choice_LR(int x, int y, int dir){
  int i,j;
  int check1=0, check2=0;
	int orientation, direction;

  //Quickly find which direction we are looking at
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
	      check1+=empty_cnt(i,j);
 		  }
		  for(j=x; j<L; j++){
  			check2+=empty_cnt(i,j);
      }
    }
	//HORIZONTAL SPAN
	}else{
		for(j=0; j<L; j++){
      for(i=0; i<y; i++){
        check1+=empty_cnt(i,j);
      }
      for(i=y; i<H; i++){
        check2+=empty_cnt(i,j);
      }
    }
	}

  //Decide which half of the map is less explored based on the two counters
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

/*
 * Function: map_average
 * --------------------
 * Squeezes the 1x1 cm rapresentation of the map into a 5x5 cm one and generates a map_copy for future image processing
 * --------------------
 * Made by Martina & Luca
 */
void map_average(){

    int row_ext, col_ext, row_int, col_int, k;
    int average[MAP_SQUARE][MAP_SQUARE]={{0}};
    int average_square=0;
    int flag=0;

    //for all groups of 5 rows and cols in the 1x1 matrix
    for (row_ext = P+H+P; row_ext > 0; row_ext-=5) {
      for (col_ext = 0; col_ext < P+L+P; col_ext+=5) {
        for(row_int=row_ext-MAP_SQUARE; row_int<row_ext; row_int++){
          for(col_int=col_ext; col_int < col_ext+MAP_SQUARE; col_int++){
              //if square is a sure measure
              if((mat[row_int][col_int] & 0b11)==0b01){
                //if square is a sure hit average of the 1x1cm square = 8
                //if square is a sure miss average of the 1x1cm square = -8
                if((((mat[row_int][col_int] >> 2) & 0b11))==HIT){
                  average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=8;
                  flag=1;
                } else {
                  average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=-8;
                  flag=1;
                }
              } else {
                //else for all readings in a square if it's a hit average+=1
                // if it's a miss average-=1
                for(k=0; k<8; k++){
                  if((((mat[row_int][col_int] >> (2*k)) & 0b11))==HIT){
                    average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]+=1;
                    flag=1;
                  } else if((((mat[row_int][col_int] >> (2*k)) & 0b11))==MISS){
                  }
                }
              }
                  if(flag==0){
                    average_square=0;
                  }
                  //average of the 5x5cm square is increased by 1 if average
                  //of the 1x1cm square is larger than 0, is decreased by 1
                  //otherwise
                  else if(average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]>=0){
                    average_square+=1;
                    } else {
                      average_square-=1;
                    }
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]=0;
                  flag=0;
              }
          }
        //if the majority of smaller squares are filled, the 5x5cm square is //set as an obstacle
        if(average_square>0 && average_square<=25){
          printf("@ ");
          map_copy[row_ext/5-1][col_ext/5] = '@';
        //if the majority of smaller squares are empty, the 5x5cm square is //set as empty
        } else if (average_square<0) {
            printf("_ ");
            map_copy[row_ext/5-1][col_ext/5] = '_';
        //otherwise is left unknown
        } else {
          printf("? ");
          map_copy[row_ext/5-1][col_ext/5] = '?';
        }

        average_square=0;
        }
          printf("\n");
      }
    return;

}

/*
 * Function: map_average_w
 * --------------------
 * Squeezes the 1x1 cm rapresentation of the map into a 5x5 cm one by giving a w wieght to the MISS lecture.
 * It generates also a map_copy of it for future image processing
 * —------------------
 * w: weight of the MISS lecture
 * —------------------
 * Made by Martina & Luca
 */
void map_average_w(float w){
  int row_ext, col_ext, row_int, col_int, k;
  float average[MAP_SQUARE][MAP_SQUARE]={{0}};
  int average_square=0;
  int flag=0;

  //Same comments as the previous function
  for (row_ext = P+H+P; row_ext > 0; row_ext-=5) {
    for (col_ext = 0; col_ext < P+L+P; col_ext+=5) {
      for(row_int=row_ext-MAP_SQUARE; row_int<row_ext; row_int++){
        for(col_int=col_ext; col_int < col_ext+MAP_SQUARE; col_int++){
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
                if((((mat[row_int][col_int] >> (2*k)) & 0b11))==HIT){
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]+=1;
                  flag=1;
                } else if((((mat[row_int][col_int] >> (2*k)) & 0b11))==MISS){
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]-=w;
                  flag=1;
                }
              }
            }
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
      if(average_square>0 && average_square<=25){
        printf("@ ");
        map_copy[row_ext/5-1][col_ext/5] = '@';
      } else if (average_square<0) {
          printf("_ ");
          map_copy[row_ext/5-1][col_ext/5] = '_';
      } else {
        printf("? ");
        map_copy[row_ext/5-1][col_ext/5] = '?';
      }
      average_square=0;
      }
        printf("\n");
    }
  return;
}

/*
 * Function: print_matrix
 * --------------------
 * Displays on the sreen the averaged matrix
 * —------------------
 * matrix: matrix to be displayed
 * —------------------
 * Made by Luca
 */
void print_matrix(int matrix[H_AVG][L_AVG]){
  int i,j;
  for (i = H_AVG-1; i >= 0; i-=1) {
    for (j = 0; j < L_AVG; j+=1) {
        printf("%c ",matrix[i][j]);
    }
    printf("\n");
  }
}

/*
 * Function: image_proc
 * --------------------
 * Apply some transformations to the map in order to eliminate the unexplored areas
 * —------------------
 * full: character used to mark an obstacle in the map
 * empty: character used to mark an empty location in the map
 * boh: character used to mark an unknown location in the map
 * map_proc: map on which apply the transformations
 * —------------------
 * Made by Luca
 * TODO eliminate print_matrix in the middle
 * TODO decide how to work on the map
 */

void image_proc(int full,int empty,int boh,int map_proc[H_AVG][L_AVG]){
  int num_row = H_AVG, num_col = L_AVG;
  int iterate=1;
  int full_num=0;
  int empty_num=0;
  int boh_num=0;
  //int new_mat[H_AVG][L_AVG];
  int row,col;

  // eliminate the unknown locations of the map surrounded by well known points
  // iterate until at least one change is performed in the map
  while (iterate){
  iterate=0;
  for (row=0;row<num_row;row++){
    for (col=0;col<num_col;col++){
      // new_mat[row][col]=map_proc[row][col];
      // to avoid segmentation fault and the full flooding given by the walls
      if ((row>2)&&(col>2)&&(row<num_row-3)&&(col<num_col-3)){
        if (map_proc[row][col]==boh){
          if (map_proc[row+1][col]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row+1][col]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row][col+1]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row][col+1]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row-1][col]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row-1][col]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row][col-1]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row][col-1]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }
          // number of full around the unknown location is grater than the empty spaces and unknown one
          if ((full_num >= empty_num)&&(full_num >= boh_num)) {
            map_proc[row][col]=full;
            //new_mat[row][col]=full;
            iterate=1;
          }
          // number of empty around the unknown location is grater than the full spaces and unknown one
          if ((empty_num > full_num)&&(empty_num >= boh_num)) {
            map_proc[row][col]=empty;
            //new_mat[row][col]=empty;
            iterate=1;
          }
          empty_num=0;
          boh_num=0;
          full_num=0;
        }
      }
     }
    }
    /*for (row=0;row<num_row;row++){
      for (col=0;col<num_col;col++){
        map_proc[row][col]=new_mat[row][col];
      }
    }
    */
  }

  printf("\n\n");
  print_matrix(map_proc);

  empty_num=0;
  boh_num=0;
  full_num=0;
  // substitute the empty spaces surrounded by more than 3 full with a full one
  for (row=0;row<num_row;row++){
    for (col=0;col<num_col;col++){
      if ((row>1)&&(col>1)&&(row<num_row-2)&&(col<num_col-2)){
        if (map_proc[row][col]==empty){
          if (map_proc[row+1][col]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row+1][col]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row][col+1]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row][col+1]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row-1][col]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row-1][col]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row][col-1]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row][col-1]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (full_num >= 3){
            map_proc[row][col]=full;
          }
          empty_num=0;
          boh_num=0;
          full_num=0;
        }
        // substitute all the unknown points with empty one
        else {
          if (map_proc[row][col]==boh){
            map_proc[row][col]=empty;
          }
        }
      }
    }
  }

  printf("\n\n");
  print_matrix(map_proc);

  empty_num=0;
  boh_num=0;
  full_num=0;
  //substitute the full spaces surround by more than 3 full with an empty one
  for (row=0;row<num_row;row++){
    for (col=0;col<num_col;col++){
      if ((row>1)&&(col>1)&&(row<num_row-2)&&(col<num_col-2)){
        if (map_proc[row][col]==full){
          if (map_proc[row+1][col]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row+1][col]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row][col+1]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row][col+1]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row-1][col]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row-1][col]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (map_proc[row][col-1]==boh){
            boh_num++;
          }
          else {
            if (map_proc[row][col-1]==empty){
              empty_num++;
            }
            else {
              full_num++;
            }
          }

          if (empty_num >= 3){
            map_proc[row][col]=empty;
          }
          empty_num=0;
          boh_num=0;
          full_num=0;
        }
      }
    }
  }

  printf("\n");
  printf("before any tunnel check\n");
  print_matrix(map_proc);

  // find the vertical corridors in the map and try to eliminate the ones with width lower than a defined threshold
  int col_num;
  int fill;
  int tunnel_cnt=0;
  for (row=0;row<num_row;row++){
    for (col=0;col<num_col;col++){
      if (map_proc[row][col]==empty){ // may be a corridor
        for (col_num=col;col_num<num_col;col_num++){
          if (map_proc[row][col_num]==full) break; // end of the corridor
          else tunnel_cnt++;
        }
        if (tunnel_cnt<2){ // 1=5cm width corridor 2=10cm width corridor
          for (fill=0;fill<tunnel_cnt;fill++){
            map_proc[row][col+fill]=full; // corridor found place some fulls to fill it
          }
        }
        tunnel_cnt=0;
        col = col_num;
      }
    }
  }

  printf("\n");
  printf("after orizzontal tunnel check\n");
  print_matrix(map_proc);

  // find the orizontal corridors in the map and try to eliminate the ones with width lower than a defined threshold
  int row_num;
  tunnel_cnt=0;
  for (col=0;col<num_col;col++){
    for (row=0;row<num_row;row++){
      if (map_proc[row][col]==empty){ // may be a corridor
        for (row_num=row;row_num<num_row;row_num++){
          if (map_proc[row_num][col]==full) break; // end of the corridor
          else tunnel_cnt++;
        }
        if (tunnel_cnt<2){ // 1=5cm width corridor 2=10cm width corridor
          for (fill=0;fill<tunnel_cnt;fill++){
            map_proc[row+fill][col]=full; // corridor found place some fulls to fill it
          }
        }
        tunnel_cnt=0;
        row = row_num;
      }
    }
  }

  printf("\n");
  printf("after vertical tunnel check\n");
  print_matrix(map_proc);

 }
