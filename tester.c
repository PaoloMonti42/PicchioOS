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

  pthread_create( &logger, NULL, position_logger, NULL );

  sensor_init( &touch, &color, &compass, &gyro, &dist );

	motor_init( &motors[0], &motors[1] );

	set_sensor_mode_inx(gyro, GYRO_GYRO_RATE);
	set_sensor_mode_inx(gyro, GYRO_GYRO_ANG);
	set_sensor_mode_inx(color, COLOR_RGB_RAW);


	go_forwards_cm(motors, 10, MAX_SPEED/8);
	wait_motor_stop(motors[0]);
	wait_motor_stop(motors[1]);

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
		set_tacho_command_inx( *motor0, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_command_inx( *motor0, TACHO_STOP );
	}
	if ( !ev3_search_tacho_plugged_in( MOT_DX, 0, motor1, 0 )) {
		fprintf( stderr, "Motor DX not found!\n" );
		set_tacho_command_inx( *motor1, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_command_inx( *motor1, TACHO_STOP );
	}
	return all_ok;
}

void *position_logger(void *arg)
{
	FILE* fp = fopen("log.txt", "w+");
	int i;
	for ( ; ; ) {
	  fprintf( fp, "ID = %d    x = %+.4d    y = %+.4d    dir = %.3d\n", MY_ID, my_pos.x, my_pos.y, my_pos.dir );
		millisleep(100);
  }
	fprintf( stdout, "Finished logging!\n" );
	fclose( fp );
	return NULL;
}
