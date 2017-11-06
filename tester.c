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
  // } 

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
