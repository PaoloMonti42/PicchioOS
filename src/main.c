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


int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *gyro, uint8_t *dist);
int motor_init(uint8_t *motor0, uint8_t *motor1, uint8_t *motor_obs, uint8_t *motor_head);
void *position_logger(void *arg);
void * bt_client(void *arg);
void *direction_updater(void *arg);
void *position_updater(void *arg);
void *gyro_position_updater(void *arg);
void *wait_kick(void *arg);
static void kill_all(int signo);

int offset = 0;
int flag_killer=0;
uint8_t motor_obs;
uint8_t motor_head;
uint8_t motors[2];
pthread_t logger;
pthread_t client;
pthread_t direction;
pthread_t position_thread;
pthread_t obstacle_thread;
pthread_mutex_t gyro_lock;
pthread_mutex_t pos_lock;

int main( int argc, char **argv )
{
	(void)(argv);
	int i;
	char buf[256];
	uint8_t touch;
	uint8_t color;
	uint8_t gyro;
	uint8_t dist;
	struct thread_arguments thread_args;
	struct obstacle_thread_arguments obs_args;

	int turns, released, timeout, rot_th, bluetooth;
	int d, rev;
	struct timeb t0, t1;
	int count = 0, flag = 0, rand_th = 0;
	float prevX, prevY, newX, newY;

	int	obstacles[9];
	int angles[9];
	int turn;
	int recal_flag = 0;
	srand(time(NULL));

	//Initialization check
  if ( ev3_init() == -1 )
	  return ( 1 );

	//Setting up a signal handler to stop the robot with a Ctrl+C
	if (signal(SIGINT, kill_all) == SIG_ERR) {
        	fputs("An error occurred while setting a signal handler.\n", stderr);
        	return EXIT_FAILURE;
   	 }

	picchio_greet();
	printf( "*** ( PICCHIO ) Hello! ***\n" );

  //Initializing sensors and motors
	sensor_init( &touch, &color, &gyro, &dist );
	motor_init( &motors[0], &motors[1], &motor_obs, &motor_head );

	my_pos.dir = START_DIR;
	//Run with default settings
	if (argc > 1) {
		my_pos.x = START_X+P;
		my_pos.y = START_Y+P;
		gyro_pos.x = START_X+P;
		gyro_pos.y = START_Y+P;
		released = 1;
		rot_th = ROT_THRESHOLD;
		bluetooth = 0;
		start = 1;
		turns = 10;
		timeout = -1;
		add_small_arena_walls();

	} else {
		//Take settings from standard input
		int tmp;
		printf("What is my starting x coordinate? \n");
		scanf("%d", &tmp);
		my_pos.x = tmp+P;
		gyro_pos.x = tmp+P;

		printf("What is my starting y coordinate? \n");
		scanf("%d", &tmp);
		my_pos.y = tmp+P;
		gyro_pos.y = tmp+P;

		printf("Will I release the obstacle? \n");
		scanf("%s", buf);
		if (buf[0] == 'y' || buf[0] == 'Y') {
			released = 0;
			rot_th = ROT_THRESHOLD+5;
		} else {
			released = 1;
			rot_th = ROT_THRESHOLD;
		}

		printf("Will I connect to the bluetooth server? \n");
		scanf("%s", buf);
		if (buf[0] == 'y' || buf[0] == 'Y') {
			bluetooth = 1;
			start = 0;
		} else {
			bluetooth = 0;
			start = 1;
		}

		printf("Am I in the large arena? \n");
		scanf("%s", buf);
		if (buf[0] == 'y' || buf[0] == 'Y') {
			add_large_arena_walls();
		} else {
			add_small_arena_walls();
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

	//Spawn threads and set mutexes up
	pthread_create( &logger, NULL, position_logger, NULL);

	if(bluetooth) {
		pthread_create( &client, NULL, bt_client, NULL );
	}

	pthread_mutex_init(&gyro_lock, NULL);
	pthread_create( &direction, NULL, direction_updater, (void *)&gyro);

	pthread_mutex_init(&pos_lock, NULL);
	thread_args.motor0 = motors[0];
	thread_args.motor1 = motors[1];
	pthread_create( &position_thread, NULL, position_updater, (void *)&thread_args);

	//Wait for the start message
	while(!start);

	//Start the timeout clock
	ftime(&t0);

	if (argc == 3) {
		//rotation recalibration
		turn_right_motors(motors, MAX_SPEED/16, 360);
		wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
		millisleep(2000);
		turn_left_motors(motors, MAX_SPEED/16, 360);
		wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
		return 0;
	}


	if (argc == 4) {
		//Useful for random tests
		return 0;
	}

	for (i = 0; i < turns; i++) {

		//We enter this condition, randomly, after the first few turns, in order to better explore new areas
		if (i > 3 && rand()%10 >= rand_th && flag>=2) {

			count = count+turn;
			if (turn > 0) {
				turn_right_motors(motors, MAX_SPEED/16, 90);
			} else {
				turn_left_motors(motors, MAX_SPEED/16, 90);
			}

			//Safely reset the gyroscope
			millisleep(50);
			pthread_mutex_lock(&gyro_lock);
			offset = gyro_pos.dir;
			set_gyro(gyro);
			pthread_mutex_unlock(&gyro_lock);
			millisleep(50);

			//If we still haven't dropped the obstacle, and we traveled enough to have space to do it
			if (d > 40 && !released) {
				rand_th = 5;
				go_forwards_cm(motors, 10, MAX_SPEED/4);
				obs_args.motor = motor_obs;
				obs_args.motor0 = motors[0];
				obs_args.motor1 = motors[1];
				obs_args.speed = MAX_SPEED/16;
				obs_args.pos_down = -60;
				obs_args.pos_up = 0;
				if (bluetooth) {
					send_obs();
				}
				//Spawn the thread to manage the release of the obstacle
				pthread_create(&obstacle_thread, NULL, release_obs_routine, (void *)&obs_args);
				sleep(4);
				d = d - 15;
				released = 1;
				//Update the dimension of the robot for better turning
				rot_th -= ROT_THRESHOLD;
			}

			d = d/2;
			//Save the coordinates before starting to move
			prevX = my_pos.x; prevY = my_pos.y;
			//Go until an obstacle, keep track if we bumped or not
			int bump = go_forwards_cm_obs(motors, motor_head, dist, touch, d, 12, MAX_SPEED/4);
			//Check if our direction drifted. If yes, recalibtrate
			int panicked = panic(motors, &pos_lock);
			//Save the coordinates after stopping
			newX = my_pos.x; newY = my_pos.y;
			//Compute the distance between the start and the stop point
			d = (int)point_distance(prevX, prevY, newX, newY);
			if (!panicked) {
				//If we drifted, we should not trust our path so we don't update the map
				map_fix(prevX, prevY, my_pos.dir, d+FACE, ROBOT_WIDTH, SURE_MISS);
			}
			if (bump && d > 10) {
				//If we bumped, we go backwards a bit to get room
				go_backwards_cm(motors, 10, MAX_SPEED/4);
		  }
			//We decide which half of the map is the least explored and we turn towards it
			turn = choice_LR((int)my_pos.x, (int)my_pos.y, my_pos.dir);
			count = count+turn;
			if (turn > 0) {
				turn_right_motors(motors, MAX_SPEED/16, 90);
			} else {
				turn_left_motors(motors, MAX_SPEED/16, 90);
			}

			//Safely reset the gyroscope
			millisleep(50);
			pthread_mutex_lock(&gyro_lock);
			offset = gyro_pos.dir;
			set_gyro(gyro);
			pthread_mutex_unlock(&gyro_lock);
			millisleep(50);

			flag = 0;

		} else {
			//Save the coordinates before starting to move
			prevX = my_pos.x; prevY = my_pos.y;
			//Go until an obstacle, keep track of the direction of the head
			int head_pos = go_forwards_obs(motors, motor_head, dist, touch, 7, MAX_SPEED/4);
			//Check if our direction drifted. If yes, recalibtrate
			int panicked = panic(motors, &pos_lock);
			millisleep(100);
			//Check whether or not we stopped in front of a movable obstacle
			check_ball(dist, color, my_pos.dir);
			//Save the coordinates after stopping
			newX = my_pos.x; newY = my_pos.y;
			//Compute the distance between the start and the stop point
			d = (int)point_distance(prevX, prevY, newX, newY);
			if (!panicked) {
				//If we drifted, we should not trust our path so we don't update the map
				map_fix(prevX, prevY, my_pos.dir, d+FACE, ROBOT_WIDTH, SURE_MISS);
			}
			if (abs(head_pos) < 40 && recal_flag == 1) {
				//If the obstacle is in front of us, but we are not straight, we recalibrate with it
				angle_recal(motors, motor_head, dist, gyro, 15, 6, 20, &gyro_lock);
		  }
			recal_flag = 0;

			int do_rev = 1;
			while(1) {
				//Scan surroundings and update the map with results
				scan_for_obstacle_N_pos_head(motor_head, dist, obstacles, angles, 7, 125, MAX_SPEED/16);
				update_map(my_pos.x, my_pos.y, my_pos.dir, 7, obstacles, angles);

				//Based on surroundings, decide the best way to move
				if (obstacles[0] != 0 && obstacles[0] < rot_th && obstacles[6] != 0 && obstacles[6] < rot_th) {
					// blocked in both directions, go back by <rev> cm
					rev = 15;
					go_backwards_cm(motors, rev, MAX_SPEED/4);
					do_rev = 0;
				} else if (obstacles[0] != 0 && obstacles[0] < rot_th) {
					// blocked to the left, turn left, go back a bit, then realign
					rev = 10;
					turn_left_motors(motors, MAX_SPEED/16, 45);
					go_backwards_cm(motors, rev, MAX_SPEED/4);
					turn_right_motors(motors, MAX_SPEED/16, 45);
					do_rev = 0;
				} else if (obstacles[6] != 0 && obstacles[6] < rot_th) {
					// blocked to the right, turn right, go back a bit, then realign
					rev = 10;
					turn_right_motors(motors, MAX_SPEED/16, 45);
					go_backwards_cm(motors, rev, MAX_SPEED/4);
					turn_left_motors(motors, MAX_SPEED/16, 45);
					do_rev = 0;
				} else {
					if (do_rev) {
						//Not blocked, just go back a small distance
						rev = 5;
						go_backwards_cm(motors, rev, MAX_SPEED/4);
					}
					break;
				}
				//Check if we drifted just in case
				panic(motors, &pos_lock);
			}

			//We decide which half of the map is the least explored and we turn towards it
			turn = choice_LR((int)my_pos.x, (int)my_pos.y, my_pos.dir);
			count += turn;
			if (turn > 0) {
				turn_right_motors(motors, MAX_SPEED/16, 90);
			} else {
				turn_left_motors(motors, MAX_SPEED/16, 90);
			}

			//Safely reset the gyroscope
			millisleep(50);
			pthread_mutex_lock(&gyro_lock);
			offset = gyro_pos.dir;//((count*90 + 180) % 360 + 360) % 360 - 180;
			set_gyro(gyro);
			pthread_mutex_unlock(&gyro_lock);
			millisleep(50);

			flag++;
		}
		ftime(&t1);
		//Check timeout clock
		if (timeout > 0 && t1.time-t0.time > timeout)
		 	break;
	}

	//Map processing and printing
	map_print(0, 0, P+L+P, P+H+P);
	map_average();
	image_proc('@','_','?',map_copy);

	if (bluetooth) {
		// send_map();
		print_matrix(map_copy);
		send_matrix(map_copy);
		//wait_stop(); //useless?
	}

	//Uninitilize and return
	ev3_uninit();
	printf( "*** ( PICCHIO ) Bye! ***\n" );
	return ( 0 );
}

/*
 * Function: sensor_init
 * --------------------
 * Search the sensors and initialize them
 * --------------------
 * touch: handle of the touch sensor
 * color: handle of the color sensor
 * gyro: handle of the gyroscope module
 * dist: handle of the distance sensor
 * --------------------
 * return: whether or not the inizialitazion was successful
 * --------------------
 * Made by Paolo
 */
int sensor_init(uint8_t *touch, uint8_t *color, uint8_t *gyro, uint8_t *dist) {
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

/*
 * Function: motor_init
 * --------------------
 * Search the motors and initialize them
 * --------------------
 * motor0: handle of the left wheel motor
 * motor1: handle of the right wheel motor
 * motor_obs: handle of the obstacle motor
 * motor_head: handle of the head motor
 * --------------------
 * return: whether or not the inizialitazion was successful
 * --------------------
 * Made by Paolo
 */
int motor_init(uint8_t *motor0, uint8_t* motor1, uint8_t* motor_obs, uint8_t *motor_head) {
	int all_ok = 1;
	ev3_tacho_init();
	if ( !ev3_search_tacho_plugged_in( MOT_SX, 0, motor0, 0 )) {
		fprintf( stderr, "Motor SX not found!\n" );
		set_tacho_stop_action_inx( *motor0, STOP_ACTION );
		set_tacho_command_inx( *motor0, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_stop_action_inx( *motor0, STOP_ACTION );
		set_tacho_command_inx( *motor0, TACHO_STOP );
	}
	if ( !ev3_search_tacho_plugged_in( MOT_DX, 0, motor1, 0 )) {
		fprintf( stderr, "Motor DX not found!\n" );
		set_tacho_stop_action_inx( *motor1, STOP_ACTION );
		set_tacho_command_inx( *motor1, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_stop_action_inx( *motor1, STOP_ACTION );
		set_tacho_command_inx( *motor1, TACHO_STOP );
	}
	if ( !ev3_search_tacho_plugged_in( MOT_OBS, 0, motor_obs, 0 )) {
		fprintf( stderr, "Motor OBS not found!\n" );
		set_tacho_stop_action_inx( *motor_obs, TACHO_HOLD );
		set_tacho_command_inx( *motor_obs, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_stop_action_inx( *motor_obs, STOP_ACTION );
		set_tacho_command_inx( *motor_obs, TACHO_STOP );
		set_tacho_position( *motor_obs, 0 );
	}
	if ( !ev3_search_tacho_plugged_in( MOT_HEAD, 0, motor_head, 0 )) {
		fprintf( stderr, "Motor OBS not found!\n" );
		set_tacho_stop_action_inx( *motor_head, STOP_ACTION );
		set_tacho_command_inx( *motor_head, TACHO_STOP );
		all_ok = 0;
	} else {
		set_tacho_stop_action_inx( *motor_head, STOP_ACTION );
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

/*
 * Function: direction_updater
 * --------------------
 * Update the direction seen by the gyroscope by polling it
 * --------------------
 * arg: handle of gyroscope module
 * --------------------
 * Made by Paolo
 */
void *direction_updater(void *arg)
{
	uint8_t gyro = * (uint8_t *) arg;
	int samples = 5;
	int d;
	for ( ; flag_killer==0; ) {
		pthread_mutex_lock(&gyro_lock);
	  d = (int)get_value_samples(gyro, samples);
		d = (( (d + offset) % 360 ) + 360 ) % 360;
		if (d > 180) {
			d = d - 360;
		}
		gyro_pos.dir = d;
		pthread_mutex_unlock(&gyro_lock);
		millisleep(10);
  }
	return NULL;
}

/*
 * Function: position_updater
 * --------------------
 * Update the position of the robot by looking at the wheel motor speeds
 * --------------------
 * thread_args: pointer to the struct with the handle to the motor array
 * --------------------
 * Made by Paolo
 */
void *position_updater(void * thread_args)
{
	//Retrieve the motor handlers from the input parameter
	struct thread_arguments * args;
	args = (struct thread_arguments *) thread_args;
	uint8_t motors[2];
	motors[0]=args->motor0;
	motors[1]=args->motor1;

	float dx = 0, dir, dt;
	int sp0, sp1;
	struct timeb t0, t1;

  //Start the timers
	ftime(&t0); ftime(&t1);

	for ( ; flag_killer==0; ) {
		pthread_mutex_lock(&pos_lock);
		//Read the motor speeds
	  get_tacho_speed(motors[0], &sp0);
		get_tacho_speed(motors[1], &sp1);
		sp0 = sp0 * MOT_DIR;
		sp1 = sp1 * MOT_DIR;
		ftime(&t1);
		if ((sp0 > 0 && sp1 > 0) || (sp0 < 0 && sp1 < 0)) {
			//If they have the same sign, it means that the robot is moving and not turning
			float speed = (sp0 + sp1) / 2.0;
			//Compute elapsed time since last iteration
			dt = (t1.time-t0.time)*1000+(t1.millitm-t0.millitm);
			//Compute distance difference
			dx = (speed*M_PI)/180*WHEEL_RADIUS*0.1*dt*1.0;
			dir = my_pos.dir;
			//Compute new coordinates and check for OOB
			float tx = my_pos.x + dx * sin((dir * M_PI) / 180.0);
			if (tx < P) {
				my_pos.x = P + ROBOT_WIDTH/2;
			} else if (tx > P + L) {
				my_pos.x = P + L - ROBOT_WIDTH/2;
			} else {
				my_pos.x = tx;
			}
			float ty = my_pos.y + dx * cos((dir * M_PI) / 180.0);
			if (ty < P) {
				my_pos.y = P + ROBOT_WIDTH/2;
			} else if (ty > P + H) {
				my_pos.y = P + H - ROBOT_WIDTH/2;
			} else {
				my_pos.y = ty;
			}

			//Same for gyro coordinates
			dir = gyro_pos.dir;
			tx = gyro_pos.x + dx * sin((dir * M_PI) / 180.0);
			if (tx < P) {
				gyro_pos.x = P + ROBOT_WIDTH/2;
			} else if (tx > P + L) {
				gyro_pos.x = P + L - ROBOT_WIDTH/2;
			} else {
				gyro_pos.x = tx;
			}
			ty = gyro_pos.y + dx * cos((dir * M_PI) / 180.0);
			if (ty < P) {
				gyro_pos.y = P + ROBOT_WIDTH/2;
			} else if (ty > P + H) {
				gyro_pos.y = P + H - ROBOT_WIDTH/2;
			} else {
				gyro_pos.y = ty;
			}
		}
		t0 = t1;
		pthread_mutex_unlock(&pos_lock);
		millisleep(10);
  }
	return NULL;
}

/*
 * Function: position_logger
 * --------------------
 * Log position and direction by appending them to a file for better debugging
 * --------------------
 * Made by Paolo
 */
void *position_logger(void *arg)
{
	(void)(arg);
	FILE* fp = fopen("logs/log.txt", "w+");
	for ( ; flag_killer==0; ) {
	  fprintf( fp, "ID = %d    x = %+.4f    y = %+.4f    dir = %.3f    gyro_x = %+.4f    gyro_y = %+.4f    gyro_dir = %.3f\n", MY_ID, my_pos.x, my_pos.y, my_pos.dir, gyro_pos.x, gyro_pos.y, gyro_pos.dir);
		millisleep(100);
  }
	fprintf( stdout, "Finished logging!\n" );
	fclose( fp );
	return NULL;
}

/*
 * Function: bt_client
 * --------------------
 * Manage bluetooth communication
 * --------------------
 * Made by Paolo
 */
void *bt_client(void *arg)
{
	(void)(arg);
	pthread_t kick;
	printf("[BT] - Bluetooth client starting up...\n");
	while (bt_init() != 0);
	printf("[BT] - Successful server connection!\n");
	pthread_create( &kick, NULL, wait_kick, NULL );
	for ( ; flag_killer==0; ) {
	  send_pos();
		millisleep(1900);
  }
	map_average();
	image_proc('@','_','?',map_copy);
	print_matrix(map_copy);
	send_matrix(map_copy);
	// send_map();
	//wait_stop(); //useless?
	printf("[BT] - Client returning...\n");
	return NULL;
}

/*
 * Function: wait_kick
 * --------------------
 * Wait for the kick message from the server
 * --------------------
 * Made by Paolo
 */
void *wait_kick(void *arg) {
	(void)(arg);
  char type;
  char string[58];
  while(flag_killer == 0){
    read_from_server (bt_sock, string, 58);
      type = string[4];
    if (type == MSG_KICK){
      printf("[BT] - I was kicked by the server :(");
      flag_killer = 1;
    }
  }
	return NULL;
}

/*
 * Function: kill_all
 * --------------------
 * Signal handler for stopping the robot when Ctrl+C is pressed
 * --------------------
 * Made by Valerio
 */
static void kill_all(int signo) {
	(void)(signo);
  puts("Stopping the motors and pausing the threads.\n");

	stop_motor(motors[0]);
	stop_motor(motors[1]);
	stop_motor(motor_obs);
	stop_motor(motor_head);
	flag_killer=1;

	pthread_join(direction, NULL);
	pthread_mutex_destroy(&gyro_lock);
	pthread_join(position_thread, NULL);
	pthread_join(logger, NULL);
	pthread_join(client, NULL);

	map_print(0, 0, P+L+P, P+H+P);
	map_average();
	ev3_uninit();

	exit(EXIT_SUCCESS);
}
