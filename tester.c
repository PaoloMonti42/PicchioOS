#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"

#include <pthread.h>
#include <math.h>
#include "map.h"
#include "picchio_lib.h"
// #include "bt.h"



int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *compass, uint8_t *gyro, uint8_t *dist);
int motor_init(uint8_t *motor0, uint8_t *motor1, uint8_t *motor_obs, uint8_t *motor_head);
void *position_logger(void *arg);
void *bt_client(void *arg);
void *direction_updater(void *arg);
void *position_updater(void *arg);

uint8_t motors[2];
uint8_t gyro;

int main( int argc, char **argv )
{
	int i,j;
	char s[256];
	//TODO
	uint8_t motor_obs;
	uint8_t motor_head;
	uint8_t touch;
	uint8_t color;
	uint8_t compass;
	//TODO
	uint8_t dist;
	rgb color_val;
	pthread_t logger;
  pthread_t client;
	pthread_t direction;
  pthread_t position;

	char command;
	int	obstacles[180];
	int angles[9];
	int distance;
	int pos;
	int span;
	float val;
	char go;
	int turn;
	srand(time(NULL));

  if ( ev3_init() == -1 )
	  return ( 1 );
	picchio_greet();
	printf( "*** ( PICCHIO ) Hello! ***\n" );

	if (argc == 3) {
		my_pos.x = atoi(argv[1])+P;
		my_pos.y = atoi(argv[2])+P;
	} else {
		my_pos.x = START_X+P;
		my_pos.y = START_Y+P;
		my_pos.dir = START_DIR;
	}

  pthread_create( &logger, NULL, position_logger, NULL );
	// pthread_create( &client, NULL, bt_client, NULL );
	pthread_create( &logger, NULL, direction_updater, NULL );
	pthread_create( &logger, NULL, position_updater, NULL );

  sensor_init( &touch, &color, &compass, &gyro, &dist );

	motor_init( &motors[0], &motors[1], &motor_obs, &motor_head );


	add_wall(0, 0, P+L+P, P, SURE_HIT);							// bottom
  add_wall(0, 0, P, P+H+P, SURE_HIT);							// left
  add_wall(0, P+H, P+L+P, P+H+P, SURE_HIT);				// top
  add_wall(P+L, 0, P+L+P, P+H+P, SURE_HIT);				// right

	int turns, d, rev;
	int count = 0, flag = 0;
	float prevX, prevY, newX, newY;
	printf("Insert number of turns: ");
	scanf("%d", &turns);
	release_obs_routine(motor_obs, motors, MAX_SPEED/16, 0, 3.4);
	for (i = 0; i < turns; i++) {
/*
		if (i > 3 && rand()%10 >= 8 && flag >= 1) {
			count = count+turn;
			turn_to_angle(motors, gyro, MAX_SPEED/16, count*90);

			d = d/2;
			go_forwards_cm(motors, d, MAX_SPEED/2);
			map_fix(prevX, prevY, d, my_pos.dir, SURE_MISS);

			wait_motor_stop(motors[0]);
			wait_motor_stop(motors[1]);

			turn = choice_LR((int)my_pos.x, (int)my_pos.y, my_pos.dir);
			count = count+turn;
			turn_to_angle(motors, gyro, MAX_SPEED/16, count*90);

			flag = 0; //se lascio il commento, non può fare due "movimenti in mezzo" di fila
			//farei che non li poò fare, ma ha un'altra probabilità di farli =~ 80%

		} else {*/
			prevX = my_pos.x; prevY = my_pos.y;
			go_forwards_obs(motors, dist, 7, MAX_SPEED/2);
			check_ball(dist, color, my_pos.dir);
			newX = my_pos.x; newY = my_pos.y;
			d = (int)point_distance(prevX, prevY, newX, newY);
			map_fix(prevX, prevY, my_pos.dir, d, ROBOT_WIDTH, SURE_MISS);

			scan_for_obstacle_N_pos_head(motor_head, dist, obstacles, angles, 7, 160, MAX_SPEED/16);
			for (j = 0; j < 7; j++) {
				printf("%d\n", obstacles[j]);
			}

			//turn logic
			//if (bloccato in entrambi i lati) {
			if (0) {
				rev = 20;
				go_backwards_cm(motors, rev, MAX_SPEED/4);
			//} else if (bloccato a sinistra) {
			} else if (1) {
				rev = 15;
				turn_left_gyro(motors, gyro, MAX_SPEED/16, 45);
				go_backwards_cm(motors, rev, MAX_SPEED/4);
				turn_right_gyro(motors, gyro, MAX_SPEED/16, 45);
			//} else if (bloccato a destra) {
			} else if (0) {
				rev = 15;
				turn_right_gyro(motors, gyro, MAX_SPEED/16, 45);
				go_backwards_cm(motors, rev, MAX_SPEED/4);
				turn_left_gyro(motors, gyro, MAX_SPEED/16, 45);
			} else {
				rev = 5;
				go_backwards_cm(motors, rev, MAX_SPEED/4);
			}

			update_map((int)my_pos.x, (int)my_pos.y, my_pos.dir, 7, obstacles, angles);

			turn = choice_LR((int)my_pos.x, (int)my_pos.y, my_pos.dir);
			if (turn == 1) {
			  turn_right_gyro(motors, gyro, MAX_SPEED/16, 90);
			} else {
				turn_left_gyro(motors, gyro, MAX_SPEED/16, 90);
			}
			// count = count+turn;
			// turn_to_angle(motors, gyro, MAX_SPEED/16, count*90);

			flag++;
		//}
	}
	map_print(0, 0, P+L+P, P+H+P);
	map_average(); //TODO fix
/*

	map_print(0, 0, P+L+P, P+H+P);
	map_average(); //TODO fix


	// int deg = 90;
  // while(1){
	// 	go=getchar();
	// 	printf("%d\n", deg);
  //   turn_to_angle(motors, gyro, MAX_SPEED/16, deg);
	// 	turn_left_gyro(motors, gyro, MAX_SPEED/16, deg);
		// deg += 90;
		// if (deg > 180) deg = deg - 360;

		// if(go=='u'){
		//   turn_motor_obs_to_pos_up(motor_obs, MAX_SPEED/16, 0);
		//  	wait_motor_stop(motor_obs);
		// } else if (go=='w'){
		//  	go_forwards_cm(motors, 10, 200);
		// } else if (go=='d'){
		//  	turn_motor_obs_to_pos_down(motor_obs, MAX_SPEED/16, 3);
		//  	wait_motor_stop(motor_obs);
		// } else if (go=='s'){
		//  	scan_for_obstacle_N_pos_head(motor_head, dist, obstacles, angles, 9, 160, MAX_SPEED/16);
		// 	for (i = 0; i < 9; i++) {
		// 	  printf("%d\n", obstacles[i]);
		// 	}
		// } else if (go=='r'){
		//  	realease_obs_routine(motor_obs, motors, MAX_SPEED/16, 0, 3.4);
		// 	break;
		// }
	// }

	map_print(0, 0, P+L+P, P+H+P);
  map_average();
*/
	ev3_uninit();
	printf( "*** ( PICCHIO ) Bye! ***\n" );
	return ( 0 );
}

int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *compass, uint8_t *gyro, uint8_t *dist) {
	int all_ok = 1;
	ev3_sensor_init();
	// if ( !ev3_search_sensor( LEGO_EV3_TOUCH, touch, 0 )) {
	// 	fprintf( stderr, "Touch sensor not found...\n" );
	// 	all_ok = 0;
	// }
	if ( !ev3_search_sensor( LEGO_EV3_COLOR, color, 0 )) {
		fprintf( stderr, "Color sensor not found...\n" );
		all_ok = 0;
	} else {
		set_sensor_mode_inx(*color, COLOR_RGB_RAW);
	}
	// if ( !ev3_search_sensor( HT_NXT_COMPASS, compass, 0 )) {
	// 	fprintf( stderr, "Compass not found...\n" );
	// 	all_ok = 0;
	// }
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

int motor_init(uint8_t *motor0, uint8_t* motor1, uint8_t* motor_obs, uint8_t *motor_head) {
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
		set_tacho_position( *motor_obs, 0 );
	}
	if ( !ev3_search_tacho_plugged_in( MOT_HEAD, 0, motor_head, 0 )) {
		fprintf( stderr, "Motor OBS not found!\n" );
		set_tacho_command_inx( *motor_head, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_command_inx( *motor_head, TACHO_STOP );
		set_tacho_position( *motor_head, 0 );
	}
	if (all_ok){
		stop_motors(motor0);
		stop_motors(motor1);
		stop_motors(motor_obs);
	}
	return all_ok;
}

void *direction_updater(void *arg)
{
	int samples = 5;
	int d;
	for ( ; ; ) {
	  d = (int)get_value_samples(gyro, samples);
		d = (( d % 360 ) + 360 ) % 360;
		if (d > 180) {
			d = d - 360;
		}
		my_pos.dir = d;
		millisleep(10);
  }
	return NULL;
}

void *position_updater(void *arg)
{
	int sp0, sp1;
	float dx = 0, dir, speed, dt;
	struct timeb t0, t1;
	ftime(&t0); ftime(&t1);
	for ( ; ; ) {
	  get_tacho_speed(motors[0], &sp0);
		get_tacho_speed(motors[1], &sp1);
		sp0 = sp0 * MOT_DIR;
		sp1 = sp1 * MOT_DIR;
		ftime(&t1);
		if ((sp0 > 0 && sp1 > 0) || (sp0 < 0 && sp1 < 0)) {
			float speed = (sp0 + sp1) / 2.0;
			dt = (t1.time-t0.time)*1000+(t1.millitm-t0.millitm);
			dx = (speed*M_PI)/180*WHEEL_RADIUS*0.1*dt;
			dir = my_pos.dir;
			my_pos.x += dx * sin((dir * M_PI) / 180.0);
			my_pos.y += dx * cos((dir * M_PI) / 180.0);
		}
		t0 = t1;
		millisleep(10);
  }
	return NULL;
}

void *position_logger(void *arg)
{
	FILE* fp = fopen("log.txt", "w+");
	int i;
	for ( ; ; ) {
	  fprintf( fp, "ID = %d    x = %+.4f    y = %+.4f    dir = %.3f\n", MY_ID, my_pos.x, my_pos.y, my_pos.dir );
		//printf("log!\n");
		millisleep(100);
  }
	fprintf( stdout, "Finished logging!\n" );
	fclose( fp );
	return NULL;
}

// void *bt_client(void *arg)
// {
// 	printf("Bluetooth client starting up...\n");
//   sleep(2);
// 	while (bt_init() != 0);
// 	printf("Successful server connection!\n");
// 	sleep(5);
// 	robot();
// 	for ( ; ; ) {
// 	  send_pos();
// 		millisleep(1900);
//   }
// 	printf("Client returning...\n");
// 	return NULL;
// }
