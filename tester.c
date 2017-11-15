#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"

#include <pthread.h>
#include <math.h>
#include "picchio_lib.h"

const char const *colors[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( colors ) / sizeof( colors[ 0 ])))

int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *compass, uint8_t *gyro, uint8_t *dist);
int motor_init(uint8_t *motor0, uint8_t *motor1);
void *position_logger(void *arg);

int main( void )
{
	int i;
	char s[256];
	uint8_t motors[2];
	uint8_t touch;
	uint8_t color;
	uint8_t compass;
	uint8_t gyro;
	uint8_t dist;
	position my_pos = { .x = START_X, .y = START_Y, .dir = START_DIR };
	rgb color_val;
  pthread_t logger;

	char command;
	int	obstacle[180];
	int distance;
	int pos;
	int span;

  if ( ev3_init() == -1 )
	  return ( 1 );
	printf( "*** ( PICCHIO ) Hello! ***\n" );

  pthread_create( &logger, NULL, position_logger, &my_pos );

  sensor_init( &touch, &color, &compass, &gyro, &dist );

	motor_init( &motors[0], &motors[1] );

  int dir_offset = get_compass_value_samples( compass, 20 );

	set_sensor_mode_inx(gyro, GYRO_GYRO_RATE);
	set_sensor_mode_inx(gyro, GYRO_GYRO_ANG);
	set_sensor_mode_inx(color, COLOR_RGB_RAW);

	float gyro_val;
	float start_angle;

	stop_motors(motors);

	start_angle = get_value_single(gyro);
	printf("starting position gyro %f\n", start_angle);
	//printf("paolo suca \n");
	//scanf("%c", &command);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 60.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 200.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 270.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 30.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 270.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 200.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 170.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 30.0);
	// millisleep(500);
	// turn_to_angle(motors,gyro, MAX_SPEED/16, 0.0);
	// millisleep(500);

	//turn_to_angle(motors,gyro, MAX_SPEED/16, 240.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 180.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 60.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 90.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 30.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 180.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 270.0);
	//millisleep(500);
	//turn_to_angle(motors,gyro, MAX_SPEED/16, 0.0);
	//millisleep(500);


	 while (1) {
		printf("a for scan all posistion s for only front pos h for head q for quit: ");
		scanf("%c", &command);
		if (command == 'q') {
			break;
		}
		if (command == 'a') {
			printf("span for scan: ");
			scanf("%d", &span);
			printf("number of pos: ");
			scanf("%d", &pos);
			scan_for_obstacle_N_pos(motors, dist, gyro, obstacle, pos, span);
			for (i=0;i<pos;i++) {
				printf("pos %d ", i);
				if (obstacle[i] == 0)
					printf("no obstacle\n");
				else
					printf("obstacle at %d mm\n", obstacle[i]);
			}
		}
		if (command == 'h') {
			printf("span for scan h: ");
			scanf("%d", &span);
			printf("number of pos h: ");
			scanf("%d", &pos);
			scan_for_obstacle_N_pos_head(motors[1], dist, obstacle, pos, span);
			for (i=0;i<pos;i++) {
				printf("h pos %d ", i);
				if (obstacle[i] == 0)
					printf("no obstacle\n");
				else
					printf("obstacle at %d mm\n", obstacle[i]);
			}
		}
		if (command == 's') {
				if ((distance = (front_obstacle(dist))) == 0)
					printf("no front obstacle\n");
				else
					printf("front obstacle at %d\n", distance);
			}
	}





/*
	printf("Gyroscope Test is on!\n");

	turn_right(motors, MAX_SPEED/4, 90);
	wait_motor_stop( motors[0] );
	wait_motor_stop( motors[1] );

	sleep(5);

	reinit_pos_gyro(motors, gyro, MAX_SPEED/16);
	wait_motor_stop( motors[0] );
	wait_motor_stop( motors[1] );

	sleep(5);

*/

/*printf("Color test is on!\n");
char main_color[6];
int valid_color;

for(;;){
	get_color_values(&color_val, color);
	printf("R=%f, G=%f, B=%f\n", color_val.r, color_val.g, color_val.b);
	sleep(1);
  valid_color = get_main_color(&color_val, main_color);
	if (valid_color != 0){
	printf( "\r(%s) \n", main_color);
	} else {
	printf("Invalid color.\n");
	}

}
*/
/*
	turn_right(motors, MAX_SPEED, 60);
	wait_motor_stop( motors[0] );
	wait_motor_stop( motors[1] );
	turn_left(motors, MAX_SPEED/8, 120);
	wait_motor_stop( motors[0] );
	wait_motor_stop( motors[1] );

	reinit_pos_gyro(motors, gyro, MAX_SPEED/16);
	wait_motor_stop( motors[0] );
	wait_motor_stop( motors[1] ); */
/*
	turn_right_gyro(motors, gyro, MAX_SPEED/4, 90);
	wait_motor_stop( motors[0] );
	wait_motor_stop( motors[1] );

	gyro_val = get_value_samples(gyro, 5);
	printf("cur_pos=%f\n", gyro_val); */
/*
	for(i=0; i<50; i++){
	gyro_val = get_value_single(gyro) - gyro_offset;
	printf("%f\n", gyro_val);
	sleep(1);
}*/



  //int dir_offset = get_compass_value_samples( compass, 20 );
//>>>>>>> martina_gyroscope

	// printf( "Starting orientation of the compass: %d degrees\n", dir_offset );

	// for ( i = 0; i < 10 ; i++) {
	// 	update_direction(&(my_pos.dir), dir_offset, compass, 5);
	// 	printf("My direction: %d\n", my_pos.dir);
	// 	millisleep(1000);
  // }

	// for ( i = 0; i < 50 ; i++) {
	// 	printf("Distance: %f\n", get_value_samples( dist, 10 ));
	// 	millisleep(500);
  // }

	//turn_right_compass(motors, compass, MAX_SPEED/10, 90);
	// for ( i = 0; i < 10 ; i++) {
	//   turn_right_compass(motors, compass, MAX_SPEED/10, 90);
	// 	update_direction(&(my_pos.dir), dir_offset, compass, 5);
  // }

	// fprintf( stdout, "Going forwards...\n" );
	// for ( i = 0; i < 2 ; i++) {
  //   go_forwards_cm(motors, 25, MAX_SPEED/2 );
	// 	update_position(&my_pos, 25);
	// 	millisleep(2000);
	// 	my_pos.dir += 90;
	  // turn_right(motors, MAX_SPEED/4, 90);
		// update_direction(&(my_pos.dir), dir_offset, compass, 5);
		// wait_motor_stop( motors[0] );
		// wait_motor_stop( motors[1] );
		// millisleep(1000);
//<<<<<<< HEAD
  // }
//=======
  // }
//>>>>>>> martina_gyroscope

	// fprintf( stdout, "Turning right...\n" );
	// turn_right_compass( motors, compass, MAX_SPEED/4, 120 );
	// wait_motor_stop( motors[0] );
	// wait_motor_stop( motors[1] );
	//
	// fprintf( stdout, "Going forwards...\n" );
	// go_forwards_cm( motors, 45, MAX_SPEED/2 );
	// wait_motor_stop( motors[0] );
	// wait_motor_stop( motors[1] );
	//
	// fprintf( stdout, "Turning left...\n" );
	// turn_left_compass( motors, compass, MAX_SPEED/4, 115 );
	// wait_motor_stop( motors[0] );
	// wait_motor_stop( motors[1] );
	//
	// fprintf( stdout, "Going forwards...\n" );
	// go_forwards_cm( motors, 40, MAX_SPEED/2 );
	// wait_motor_stop( motors[0] );
	// wait_motor_stop( motors[1] );


  //pthread_join(logger, NULL);
	ev3_uninit();
	printf( "*** ( PICCHIO ) Bye! ***\n" );
	return ( 0 );
}

int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *compass, uint8_t *gyro, uint8_t *dist) {
	int all_ok = 1;
	ev3_sensor_init();
	if ( !ev3_search_sensor( LEGO_EV3_TOUCH, touch, 0 )) {
		fprintf( stderr, "Touch sensor not found...\n" );
		all_ok = 0;
	}
	if ( !ev3_search_sensor( LEGO_EV3_COLOR, color, 0 )) {
		fprintf( stderr, "Color sensor not found...\n" );
		all_ok = 0;
	}
	if ( !ev3_search_sensor( HT_NXT_COMPASS, compass, 0 )) {
		fprintf( stderr, "Compass not found...\n" );
		all_ok = 0;
	}
	if ( !ev3_search_sensor( LEGO_EV3_GYRO, gyro, 0 )) {
		fprintf( stderr, "Gyroscope not found...\n" );
		all_ok = 0;
	}
	if ( !ev3_search_sensor( LEGO_EV3_US, dist, 0 )) {
		fprintf( stderr, "Distance sensor not found...\n" );
		all_ok = 0;
	}
	return all_ok;
}

int motor_init(uint8_t *motor0, uint8_t* motor1) {
	int all_ok = 1;
	ev3_tacho_init();
	if ( !ev3_search_tacho_plugged_in( MOT_SX, 0, motor0, 0 )) {
		fprintf( stderr, "Motor SX not found!\n" );
		all_ok = 0;
	}
	if ( !ev3_search_tacho_plugged_in( MOT_DX, 0, motor1, 0 )) {
		fprintf( stderr, "Motor DX not found!\n" );
		all_ok = 0;
	}
	// int max_speed;
	// get_tacho_max_speed( motors[0], &max_speed );
	// printf("Motor max speed = %d\n", max_speed );
	return all_ok;
}

void *position_logger(void *arg)
{
	position *pos = (position *)arg;
	FILE* fp = fopen("log.txt", "w+");
	int i;
	for ( ; ; ) {
	  fprintf( fp, "ID = %d    x = %+.4d    y = %+.4d    dir = %.3d\n", MY_ID, pos->x, pos->y, pos->dir );
		millisleep(100);
  }
	fprintf( stdout, "Finished logging!\n" );
	fclose( fp );
	return NULL;
}
