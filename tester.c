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

const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

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
  pthread_t logger;

  if ( ev3_init() == -1 )
	  return ( 1 );
	printf( "*** ( PICCHIO ) Hello! ***\n" );

  pthread_create( &logger, NULL, position_logger, &my_pos );

  sensor_init( &touch, &color, &compass, &gyro, &dist );

	motor_init( &motors[0], &motors[1] );

  int dir_offset = get_compass_value_samples( compass, 20 );
	// printf( "Starting orientation of the compass: %d degrees\n", dir_offset );

	turn_right_compass(motors, compass, MAX_SPEED/4, 90);

	// fprintf( stdout, "Going forwards...\n" );
  // go_forwards( motors, 2000, MAX_SPEED/2 );
  // wait_motor_stop( motors[0] );
	// wait_motor_stop( motors[1] );

  // printf("Going backwards...\n");
	// go_backwards(motors, 2000, max_speed/2);
	// wait_motor_stop(motors[0]);
	// wait_motor_stop(motors[1]);

	// printf("Turning right...\n");
	// turn_right(motors, max_speed/2);
	// wait_motor_stop(motors[0]);
	// wait_motor_stop(motors[1]);
	// millisleep(2000);

	// printf("Turning left...\n");
	// turn_left(motors, max_speed/2, 360);
	// wait_motor_stop(motors[0]);
	// wait_motor_stop(motors[1]);
	//millisleep(2000);

	// for (i = 0; i < 4; i++) {
	// 		printf("Going forwards...\n");
	// 	  go_forwards(motors, 1000, max_speed/2);
	// 	  wait_motor_stop(motors[0]);
	// 		wait_motor_stop(motors[1]);
	// 	  millisleep(1000);
	// 		printf("Turning left...\n");
	// 		turn_left(motors, max_speed/2);
	// 		wait_motor_stop(motors[0]);
	// 		wait_motor_stop(motors[1]);
	// 		millisleep(1000);
  // }

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
