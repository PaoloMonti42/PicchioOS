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
#include "map.h"


int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *compass, uint8_t *gyro, uint8_t *dist);
int motor_init(uint8_t *motor0, uint8_t *motor1, uint8_t *motor_obs);
void *position_logger(void *arg);


int main( int argc, char **argv )
{
	int i;
	char s[256];
	uint8_t motors[2];
	uint8_t motor_obs;
	uint8_t touch;
	uint8_t color;
	uint8_t compass;
	uint8_t gyro;
	uint8_t dist;
	rgb color_val;
  pthread_t logger;

	char command;
	int	obstacles[180];
	int angles[9];
	int distance;
	int pos;
	int span;
	float val;
	char go;

  if ( ev3_init() == -1 )
	  return ( 1 );
	printf( "*** ( PICCHIO ) Hello! ***\n" );

  pthread_create( &logger, NULL, position_logger, NULL );

  sensor_init( &touch, &color, &compass, &gyro, &dist );

	motor_init( &motors[0], &motors[1], &motor_obs );

	if (argc == 3) { // TODO test
		my_pos.x = atoi(argv[1]);
		my_pos.y = atoi(argv[2]);
	}

	go = getchar();

	while(1){
		if (go == 't'){
			turn_left(motors, MAX_SPEED/16, 360);
			millisleep(3000);
		}
		if (go == 'd') {
			turn_motor_deg(motor_obs, MAX_SPEED/8, 120);
			millisleep(1000);
		}
		if (go == 'u') {
			turn_motor_deg(motor_obs, MAX_SPEED/8, -120);
			millisleep(1000);
		}
		if (go == 'r') {
			int d;
			scanf("%d\n", &d);
			turn_right_gyro(motors, gyro, MAX_SPEED/16, d);
			millisleep((d/90)*1000);
		}
		if (go == 'l') {
			int d;
			scanf("%d\n", &d);
			turn_left_gyro(motors, gyro, MAX_SPEED/16, d);
			millisleep((d/90)*1000);
		}
		if (go == 'o') {
			printf("%d\n", front_obstacle(dist));
		}
		if (go == 'c') {
			get_color(color, s);
			printf("%s\n", s);
		}
		if (go == 'q') {
			break;
		}
		go = getchar();

		// set_gyro(gyro);
    //
		// scan_for_obstacle_N_pos(motors, dist, gyro, obstacles, angles, 9, 180);
		// for(i=0;i<9;i++){
		// 		printf("obastacle[%d]=%d\n", i, obstacles[i]);
		// 		printf("angle[%d]=%d\n", i, angles[i]);
		// }
		// update_map(100, 100, 0, 9, obstacles, angles);
	  //pthread_join(logger, NULL);
	}

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
	} else {
		set_sensor_mode_inx(*color, COLOR_RGB_RAW);
	}
	if ( !ev3_search_sensor( HT_NXT_COMPASS, compass, 0 )) {
		fprintf( stderr, "Compass not found...\n" );
		all_ok = 0;
	}
	if ( !ev3_search_sensor( LEGO_EV3_GYRO, gyro, 0 )) {
		fprintf( stderr, "Gyroscope not found...\n" );
		all_ok = 0;
	} else {
		set_sensor_mode_inx(*gyro, GYRO_GYRO_RATE);
		set_sensor_mode_inx(*gyro, GYRO_GYRO_ANG);
	}
	if ( !ev3_search_sensor( LEGO_EV3_US, dist, 0 )) {
		fprintf( stderr, "Distance sensor not found...\n" );
		all_ok = 0;
	}
	return all_ok;
}

int motor_init(uint8_t *motor0, uint8_t* motor1, uint8_t* motor_obs) {
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
	if ( !ev3_search_tacho_plugged_in( MOT_OBS, 0, motor_obs, 0 )) {
		fprintf( stderr, "Motor OBS not found!\n" );
		set_tacho_command_inx( *motor_obs, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_command_inx( *motor_obs, TACHO_STOP );
	}
	if (all_ok){
		stop_motors(motor0);
		stop_motors(motor1);
		stop_motors(motor_obs);
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
