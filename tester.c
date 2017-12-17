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
#include <signal.h>
#include "map.h"
#include "picchio_lib.h"
#include "bt.h"


int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *compass, uint8_t *gyro, uint8_t *dist);
int motor_init(uint8_t *motor0, uint8_t *motor1, uint8_t *motor_obs, uint8_t *motor_head);
void *position_logger(void *arg);
void * bt_client(void *arg);
void *direction_updater(void *arg);
void *position_updater(void *arg);
static void kill_all(int signo);

int flag_killer=0;
uint8_t motor_obs;
uint8_t motor_head;
uint8_t motors[2];
pthread_t logger;
pthread_t client;
pthread_t direction;
pthread_t position_thread;
pthread_t obstacle_thread;

int main( int argc, char **argv )
{
	int i,j;
	//LALALALAL
	char s[256];
	uint8_t touch;
	uint8_t color;
	uint8_t compass;
	uint8_t gyro;
	struct thread_arguments thread_args;
	struct obstacle_thread_arguments obs_args;
	uint8_t dist;
	rgb color_val;

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

	if (signal(SIGINT, kill_all) == SIG_ERR) {
        	fputs("An error occurred while setting a signal handler.\n", stderr);
        	return EXIT_FAILURE;
   	 }

	picchio_greet();
	printf( "*** ( PICCHIO ) Hello! ***\n" );

	sensor_init( &touch, &color, &compass, &gyro, &dist );
	motor_init( &motors[0], &motors[1], &motor_obs, &motor_head );

	int turns, released, bluetooth, timeout, rot_th;
	int d, rev;
	struct timeb t0, t1;
	int count = 0, flag = 0;
	float prevX, prevY, newX, newY;

	my_pos.dir = START_DIR;
	if (argc > 1) {
		my_pos.x = START_X+P;
		my_pos.y = START_Y+P;
		released = 1;
		rot_th = ROT_THRESHOLD;
		bluetooth = 0;
		turns = 10;
		timeout = -1;
	} else {
		int tmp;
		char c;
		printf("What is my starting x coordinate? \n");
		scanf("%d", &tmp);
		my_pos.x = tmp;

		printf("What is my starting y coordinate? \n");
		scanf("%d", &tmp);
		my_pos.y = tmp;

		printf("Will I release the obstacle? \n");
		scanf("%s", s);
		if (s[0] == 'y' || s[0] == 'Y') {
			released = 0;
			rot_th = ROT_THRESHOLD+5;
		} else {
			released = 1;
			rot_th = ROT_THRESHOLD;
		}

		printf("Will I connect to the bluetooth server? \n");
		scanf("%s", s);
		if (s[0] == 'y' || s[0] == 'Y') {
			bluetooth = 1;
		} else {
			bluetooth = 0;
		}

		printf("How many turns will I perform? \n");
		scanf("%d", &tmp);
		turns = tmp;

		printf("After how many seconds will I stop? \n");
		scanf("%d", &tmp);
		if (tmp < 0) {
			timeout = -1;
		} else {
		timeout = tmp;
		}
	}

	printf("Ok, I'm ready to start!\n");

	pthread_create( &logger, NULL, position_logger, NULL);

	if(bluetooth) {
		pthread_create( &client, NULL, bt_client, NULL );
	}

	pthread_create( &direction, NULL, direction_updater, (void *)&gyro);
	thread_args.motor0 = motors[0];
	thread_args.motor1 = motors[1];

	pthread_create( &position_thread, NULL, position_updater, (void *)&thread_args);

	ftime(&t0);

	add_wall(0, 0, P+L+P, P, SURE_HIT);							// bottom
  add_wall(0, 0, P, P+H+P, SURE_HIT);							// left
  add_wall(0, P+H, P+L+P, P+H+P, SURE_HIT);				// top
  add_wall(P+L, 0, P+L+P, P+H+P, SURE_HIT);				// right


	for (i = 0; i < turns; i++) {

		if (i > 3 && rand()%10 >= 5 && flag>=1) { //TODO evaluate rand & flag

			count = count+turn;
			turn_to_angle(motors, gyro, MAX_SPEED/16, count*90);

			if (d > 40 && !released) {
				go_forwards_cm(motors, 10, MAX_SPEED/2);
				//release_obs_routine(motor_obs, motors, MAX_SPEED/16, 0, 4);
				obs_args.motor = motor_obs;
				obs_args.motor0 = motors[0];
				obs_args.motor1 = motors[1];
				obs_args.speed = MAX_SPEED/16;
				obs_args.height_ob_down = 4;
				obs_args.height_ob_up = 0;
				pthread_create(&obstacle_thread, NULL, release_obs_routine, (void *)&obs_args);
				sleep(4);
				d = d - 15;
				released = 1;
				rot_th -= ROT_THRESHOLD;
			}

			d = d/2;
			prevX = my_pos.x; prevY = my_pos.y;
			go_forwards_cm(motors, d, MAX_SPEED/2);
			map_fix(prevX, prevY, my_pos.dir, d, ROBOT_WIDTH, SURE_MISS);

			turn = choice_LR((int)my_pos.x, (int)my_pos.y, my_pos.dir);
			count = count+turn;
			turn_to_angle(motors, gyro, MAX_SPEED/16, count*90);

			flag = 0;

		} else {
			prevX = my_pos.x; prevY = my_pos.y;
			go_forwards_obs(motors, dist, 7, MAX_SPEED/2);
			millisleep(100);
			check_ball(dist, color, my_pos.dir);
			newX = my_pos.x; newY = my_pos.y;
			d = (int)point_distance(prevX, prevY, newX, newY);
			map_fix(prevX, prevY, my_pos.dir, d, ROBOT_WIDTH, SURE_MISS);

			while(1) {
				scan_for_obstacle_N_pos_head(motor_head, dist, obstacles, angles, 7, 160, MAX_SPEED/16);
				update_map(my_pos.x, my_pos.y, my_pos.dir, 7, obstacles, angles);
				// blocked in both directions
				if (obstacles[0] != 0 && obstacles[0] < rot_th && obstacles[6] != 0 && obstacles[6] < rot_th) {
					rev = 20;
					go_backwards_cm(motors, rev, MAX_SPEED/4);
				// blocked to the left
				} else if (obstacles[0] != 0 && obstacles[0] < rot_th) {
					rev = 15;
					turn_left_gyro(motors, gyro, MAX_SPEED/16, 45);
					go_backwards_cm(motors, rev, MAX_SPEED/4);
					turn_right_gyro(motors, gyro, MAX_SPEED/16, 45);
			// blocked to the right
				} else if (obstacles[6] != 0 && obstacles[6] < rot_th) {
					rev = 15;
					turn_right_gyro(motors, gyro, MAX_SPEED/16, 45);
					go_backwards_cm(motors, rev, MAX_SPEED/4);
					turn_left_gyro(motors, gyro, MAX_SPEED/16, 45);
				} else {
					rev = 5;
					go_backwards_cm(motors, rev, MAX_SPEED/4);
					break;
				}
			}

			turn = choice_LR((int)my_pos.x, (int)my_pos.y, my_pos.dir);
			count += turn;
			millisleep(50);
			turn_to_angle(motors, gyro, MAX_SPEED/16, count*90);

			flag++; //TODO evaluate
		}
		ftime(&t1);
		if (timeout > 0 && t1.time-t0.time > timeout)
		 	break;
	}

	map_print(0, 0, P+L+P, P+H+P);
	map_average();
	if (bluetooth) {
		send_map();
	}

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
		//set_sensor_mode_inx(*color, COLOR_COL_COLOR);
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
		stop_motor(*motor0);
		stop_motor(*motor1);
		stop_motor(*motor_obs);
		stop_motor(*motor_head);
	}
	return all_ok;
}

void *direction_updater(void *arg)
{
	uint8_t gyro = * (uint8_t *) arg;
	int samples = 5;
	int d;
	for ( ; flag_killer==0; ) {
	  d = (int)get_value_samples(gyro, samples);
		d = (( d % 360 ) + 360 ) % 360;
		if (d > 180) {
			d = d - 360;
		}
		my_pos.dir = d;
		// printf("%d\n", d);
		millisleep(10);
  }
	return NULL;
}

void *position_updater(void * thread_args)
{

	struct thread_arguments * args;
	args = (struct thread_arguments *) thread_args;
	uint8_t motors[2];
	motors[0]=args->motor0;
	motors[1]=args->motor1;
	int sp0, sp1;
	float dx = 0, dir, speed, dt;
	struct timeb t0, t1;
	ftime(&t0); ftime(&t1);
	for ( ; flag_killer==0; ) {
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
	for ( ; flag_killer==0; ) {
	  fprintf( fp, "ID = %d    x = %+.4f    y = %+.4f    dir = %.3f\n", MY_ID, my_pos.x, my_pos.y, my_pos.dir );
		//printf("log!\n");
		millisleep(100);
  }
	fprintf( stdout, "Finished logging!\n" );
	fclose( fp );
	return NULL;
}


void *bt_client(void *arg)
{
	printf("[BT] - Bluetooth client starting up...\n");
  sleep(2);
	while (bt_init() != 0);
	printf("[BT] - Successful server connection!\n");
	//sleep(5);
	for ( ; flag_killer==0; ) {
	  send_pos();
		millisleep(1900);
  }
	printf("[BT] - Client returning...\n");
	return NULL;
}


static void kill_all(int signo) {

  puts("Stopping the motors and pausing the threads.\n");

	stop_motor(motors[0]);
	stop_motor(motors[1]);
	stop_motor(motor_obs);
	stop_motor(motor_head);
	flag_killer=1;

	pthread_join(direction, NULL);
	pthread_join(position_thread, NULL);
	pthread_join(logger, NULL);
	pthread_join(client, NULL);

	map_print(0, 0, P+L+P, P+H+P);
	map_average();
	send_map();
	ev3_uninit();

	exit(EXIT_SUCCESS);
}
