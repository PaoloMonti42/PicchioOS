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

void *position_logger(void *arg)
{
	position *pos = (position *)arg;
	FILE* fp = fopen("log.txt", "w+");
	int i;
	for (i = 0; i < 100; i++) {
	  fprintf(fp, "ID = %d    x = %+.4d    y = %+.4d\n", MY_ID, pos->x, pos->y);
		millisleep(100);
  }
	printf("Finished logging!\n");
	fclose(fp);
	return NULL;
}

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
	position my_pos = { .x = START_X, .y = START_Y, .dir = START_DIR};
  pthread_t logger;

  if ( ev3_init() == -1 ) return ( 1 );
	printf( "*** ( PICCHIO ) Hello! ***\n" );
  pthread_create(&logger, NULL, position_logger, &my_pos);


	while ( ev3_tacho_init() < 1 ) millisleep( 1000 );

	if ( !ev3_search_tacho_plugged_in( MOT_SX, 0, &motors[0], 0 )) {
		fprintf( stderr, "Motor SX not found!\n" );
	}
	if ( !ev3_search_tacho_plugged_in( MOT_DX, 0, &motors[1], 0 )) {
		fprintf( stderr, "Motor DX not found!\n" );
	}

	// int max_speed;
	// get_tacho_max_speed( motors[0], &max_speed );
	// printf("Motor max speed = %d\n", max_speed );

	printf("Going forwards...\n");
  go_forwards(motors, 2000, MAX_SPEED/2);
  wait_motor_stop(motors[0]);
	wait_motor_stop(motors[1]);

  // printf("Going backwards...\n");
	// go_forwards(motors, 2000, -max_speed/2);
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


  ev3_sensor_init();

	if ( !ev3_search_sensor( LEGO_EV3_TOUCH, &touch, 0 )) {
		fprintf( stderr, "Touch sensor not found...\n" );
	} else {
		for (i = 0; i < 10; i++) {
	  	printf("%f\n", get_value_samples(touch, 100));
			millisleep(500);
		}
	}

	if ( !ev3_search_sensor( LEGO_EV3_COLOR, &color, 0 )) {
		fprintf( stderr, "Color sensor not found...\n" );
	} else {
		set_sensor_command_inx(color, COLOR_COL_COLOR); //TODO
		for (i = 0; i < 10; i++) {
	    printf("%f\n", get_value_samples(color, 100));
			millisleep(500);
	  }
	}

	if ( !ev3_search_sensor( HT_NXT_COMPASS, &compass, 0 )) {
		fprintf( stderr, "Compass not found...\n" );
	} else {
		for (i = 0; i < 10; i++) {
	    printf("%f\n", get_value_samples(compass, 100));
			millisleep(500);
	  }
	}

	if ( !ev3_search_sensor( LEGO_EV3_GYRO, &gyro, 0 )) {
		fprintf( stderr, "Gyroscope not found...\n" );
	} else {
		set_sensor_command_inx(color, COLOR_COL_COLOR); //TODO ang, rate, fas, g_and_a, cal
		for (i = 0; i < 10; i++) {
	    printf("%f\n", get_value_samples(gyro, 100));
			millisleep(500);
	  }
	}

	if ( !ev3_search_sensor( LEGO_EV3_US, &dist, 0 )) {
		fprintf( stderr, "Distance sensor not found...\n" );
	} else {
		set_sensor_command_inx(dist, 0); //TODO dist_cm, dist_in, listen, si_cm, si_in, dc_cm, dc_in
		for (i = 0; i < 10; i++) {
	    printf("%f\n", get_value_samples(dist, 100));
			millisleep(500);
	  }
	}


	ev3_uninit();
	printf( "*** ( PICCHIO ) Bye! ***\n" );
	return ( 0 );
}
