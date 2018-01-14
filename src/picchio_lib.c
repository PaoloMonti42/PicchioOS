#include <string.h>
#include <math.h>

#define millisleep( msec ) usleep(( msec ) * 1000 )
#include <sys/timeb.h>

const char const *colors[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( colors ) / sizeof( colors[ 0 ])))

typedef struct position {
	volatile float x;
	volatile float y;
	volatile float dir;
} position;

position my_pos;
position gyro_pos;

typedef struct rgb {
 	float r;
 	float g;
 	float b;
 } rgb;

 struct thread_arguments{
 	uint8_t motor0;
 	uint8_t motor1;
 };

 struct obstacle_thread_arguments{
	 int motor;
	 uint8_t motor0;
   uint8_t motor1;
	 int speed;
	 float pos_up;
	 float pos_down;
 };

 /*
  * Function: picchio_greet
  * --------------------
  * Picchio greets the user when starting the program
  * --------------------
  * Made by Picchio itself
  */
 void picchio_greet() {
 	int c;
 	FILE *fp = fopen("logs/greet.txt", "r");
 	while ((c = getc(fp)) != EOF)
         putchar(c);
     fclose(fp);
 }

 /*
  * Function: point_distance
  * --------------------
  * Computes the distance between two points A and B using geometry
  * --------------------
  * Ax and Ay: coordinates of the point A
  * Bx and By: coordinates of the point B
  * --------------------
  * returns: the distance AB as a floating point number
	* --------------------
  * Made by Paolo
  */
 float point_distance (float Ax, float Ay, float Bx, float By) {
 	return sqrt( (Ax-Bx)*(Ax-Bx) + (Ay-By)*(Ay-By) );
 }

 /*
  * Function: wait_motor_stop
  * --------------------
  * Polls a motor until it stops moving
  * --------------------
  * motor: handle of the motor to check
  * --------------------
  * Made by Paolo
  */
 void wait_motor_stop(uint8_t motor) {
 	FLAGS_T state;
 	do {
 		get_tacho_state_flags( motor, &state );
 	} while ( state );
 }

 /*
  * Function: turn_motor_deg
  * --------------------
  * Turns a motor until by a certain amount of degrees,
	* relative to the current position.
	* Does not call wait_motor_stop, so it's not blocking.
  * --------------------
  * motor: handle of the motor to check
	* speed: velocity to turn the motor at
	* deg: amount of degrees to turn the motor by
  * --------------------
  * Made by Paolo
  */
void turn_motor_deg(uint8_t motor, int speed, int deg) {
	set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	set_tacho_position_sp( motor, deg );
	set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}

/*
 * Function: turn_motor_to_pos
 * --------------------
 * Turns a motor by a certain absolute degree.
 * Does not call wait_motor_stop, so it's not blocking.
 * --------------------
 * motor: handle of the motor to check
 * speed: velocity to turn the motor at
 * deg: degree to turn the motor at
 * --------------------
 * Made by Paolo
 */
void turn_motor_to_pos(uint8_t motor, int speed, int pos) {
	set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	set_tacho_position_sp( motor, pos );
	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
}

/*
 * Function: go_forwards_cm
 * --------------------
 * Moves the robot ahead by a certain distance.
 * Calls wait_motor_stop, so it's blocking.
 * --------------------
 * motors: handle of the motor array to control
 * cm: amount of centimeters to move of
 * speed: velocity to turn the motors at
 * --------------------
 * Made by Valerio
 */
void go_forwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], speed * COMP_SX);
	set_tacho_speed_sp( motors[1], speed * COMP_DX);
	multi_set_tacho_position_sp( motors, MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	wait_motor_stop(motors[0]);
	wait_motor_stop(motors[1]);
}

/*
 * Function: go_backwards_cm
 * --------------------
 * Moves the robot backwards by a certain distance.
 * Calls wait_motor_stop, so it's blocking.
 * --------------------
 * motors: handle of the motor array to control
 * cm: amount of centimeters to move of
 * speed: velocity to turn the motors at
 * --------------------
 * Made by Valerio
 */
void go_backwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], speed * COMP_SX);
	set_tacho_speed_sp( motors[1], speed * COMP_DX);
	multi_set_tacho_position_sp( motors, -MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	wait_motor_stop(motors[0]);
	wait_motor_stop(motors[1]);
}

/*
 * Function: turn_right
 * --------------------
 * Turns the robot right by moving both wheels.
 * The amount of degrees to turns the wheels of is computed
 * through a proportion with a predetermined constant.
 * Does not call wait_motor_stop, so it's not blocking.
 * --------------------
 * motors: handle of the motor array to control
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the robot of
 * --------------------
 * Made by Paolo
 */
void turn_right(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	set_tacho_position_sp( motors[0], MOT_DIR*(TURN360*deg)/360 );
	set_tacho_position_sp( motors[1], -MOT_DIR*(TURN360*deg)/360 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
}

/*
 * Function: turn_right_motors
 * --------------------
 * Wrapper for turning the robot to the right and updating the direction.
 * --------------------
 * motors: handle of the motor array to control
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the motors of
 * --------------------
 * Made by Paolo
 */
void turn_right_motors(uint8_t *motors, int speed, int deg) {
	turn_right(motors, speed, deg);
	wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
	my_pos.dir = ((((int)my_pos.dir + deg) % 360 ) + 360 ) % 360;
	if (my_pos.dir > 180) {
		my_pos.dir -= 360;
	}
}

/*
 * Function: turn_left
 * --------------------
 * Turns the robot left by moving both wheels.
 * The amount of degrees to turns the wheels of is computed
 * through a proportion with a predetermined constant.
 * Does not call wait_motor_stop, so it's not blocking.
 * --------------------
 * motors: handle of the motor array to control
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the robot of
 * --------------------
 * Made by Paolo
 */
void turn_left(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	set_tacho_position_sp( motors[0], - MOT_DIR*(TURN360*deg)/360);
	set_tacho_position_sp( motors[1], MOT_DIR*(TURN360*deg)/360 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	//update_direction(-deg);
}

/*
 * Function: turn_left_motors
 * --------------------
 * Wrapper for turning the robot to the left and updating the direction.
 * --------------------
 * motors: handle of the motor array to control
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the motors of
 * --------------------
 * Made by Paolo
 */
void turn_left_motors(uint8_t *motors, int speed, int deg) {
	turn_left(motors, speed, deg);
	wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
	my_pos.dir = ((((int)my_pos.dir - deg) % 360 ) + 360 ) % 360;
	if (my_pos.dir > 180) {
		my_pos.dir -= 360;
	}
}

/*
 * Function: stop_motor
 * --------------------
 * Sends a stop signal to a motor to immeditely stop it
 * --------------------
 * motor: handle of the motor to control
 * --------------------
 * Made by Valerio
 */
void stop_motor(uint8_t motor) {
	set_tacho_command_inx( motor, TACHO_STOP );
}

/*
 * Function: stop_motors
 * --------------------
 * Sends a stop signal to an array of motors to immeditely stop them
 * --------------------
 * motors: handle of the motor array to control
 * --------------------
 * Made by Valerio
 */
void stop_motors(uint8_t *motors) {
	multi_set_tacho_command_inx( motors, TACHO_STOP );
}

/*
 * Function: get_value_single
 * --------------------
 * Reads a value from a sensor
 * --------------------
 * sensor: handle of the sensor to read
 * --------------------
 * returns: the value read as a floating point number
 * --------------------
 * Made by Paolo
 */
float get_value_single(uint8_t sensor) {
	float val;
	get_sensor_value0( sensor, &val );
	return val;
}

/*
 * Function: get_value_samples
 * --------------------
 * Reads many values from a sensor, then yields their average
 * --------------------
 * sensor: handle of the sensor to read
 * samples: number of samples to take
 * --------------------
 * returns: the average value read as a floating point number
 * --------------------
 * Made by Paolo
 */
float get_value_samples(uint8_t sensor, int samples) {
	float val, sum = 0;
	int i;
	for (i = 0; i < samples; i++) {
		get_sensor_value0( sensor, &val );
		sum  += val;
	}
	return sum/(float)samples;
}

/*
 * Function: turn_to_angle
 * --------------------
 * Make the robot turn to any angle by using gyroscope readings.
 * --------------------
 * motors: handle of the motor array to control
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the motors of
 * --------------------
 * Made by Paolo & Valerio
 */
void turn_to_angle(uint8_t *motors, int speed, int deg) {
 	int cur_pos = gyro_pos.dir;
	// Compute the destination angle (normalized between -180 and 180)
	int dest = ((deg + 180) % 360 + 360) % 360 - 180;
	if (dest == -180) dest = 180;

 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );

	//Compute whether to turn clockwise or counterclockwise
	if (dest > cur_pos) {
		if ((dest - cur_pos) < 180 ) {
			//If we're here, we must turn clockwise
			dest -= 6;
			if (dest < -180) dest = 180;
			set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			//Until the gyroscope reading is smaller than the target angle
		 	while(gyro_pos.dir < dest);
		 	stop_motors(motors);
		}
		else {
			//If we're here, we must turn counterclockwise
			dest += 6;
			if (dest > 180) dest = 180;
			set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			//Until the gyroscope reading is greater than the target angle
			while(gyro_pos.dir < 0);
			while(gyro_pos.dir > dest);
		 	stop_motors(motors);
		}
	}
	else {
		if ((cur_pos - dest) < 180) {
			//If we're here, we must turn counterclockwise
			dest += 6;
			if (dest > 180) dest = 180;
			set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			//Until the gyroscope reading is greater than the target angle
			while(gyro_pos.dir > dest);
		 	stop_motors(motors);
		}
		else {
			//If we're here, we must turn clockwise
			dest -= 6;
			if (dest < -180) dest = 180;
			set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			//Until the gyroscope reading is smaller than the target angle
			while(gyro_pos.dir > 0);
			while(gyro_pos.dir < dest);
		 	stop_motors(motors);
		}
	}
}

/*
 * Function: set_gyro
 * --------------------
 * Function to reset the gyroscope.
 * By changing mode of operation, we get rid of any residual drift.
 * The previous value of the gyroscope should be preserved.
 * This is handled by the code calling this function.
 * --------------------
 * motors: handle of the motor array to control
 * gyro: handle of the gyroscope module
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the motors of
 * --------------------
 * Made by Martina
 */
void set_gyro(uint8_t gyro){
	set_sensor_mode_inx(gyro, GYRO_GYRO_RATE);
	set_sensor_mode_inx(gyro, GYRO_GYRO_ANG);
}

/*
 * Function: turn_left_gyro
 * --------------------
 * Turns the robot left by moving both wheels.
 * The motors will turn until the value read from the gyroscope
 * agrees with the amount of desired degrees.
 * --------------------
 * motors: handle of the motor array to control
 * gyro: handle of the gyroscope module
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the robot of
 * --------------------
 * Made by Martina
 */
void turn_left_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg) {
  float start_dir = get_value_samples( gyro, 5 );
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
 	set_tacho_speed_sp( motors[1], MOT_DIR * speed);
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );
 	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
 	float end_dir = start_dir-deg;
  while (gyro_pos.dir > end_dir);
 	stop_motors(motors);
 }

/*
 * Function: turn_right_gyro
 * --------------------
 * Turns the robot right by moving both wheels.
 * The motors will turn until the value read from the gyroscope
 * agrees with the amount of desired degrees.
 * --------------------
 * motors: handle of the motor array to control
 * gyro: handle of the gyroscope module
 * speed: velocity to turn the motors at
 * deg: amount of degrees to turn the robot of
 * --------------------
 * Made by Martina
 */
void turn_right_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg) {
 	float start_dir = get_value_samples( gyro, 5 );
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	set_tacho_speed_sp( motors[0], MOT_DIR * speed);
 	set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );
 	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
 	float end_dir = start_dir+deg;
 	while (gyro_pos.dir < end_dir);
 	stop_motors(motors);
 }

/*
 * Function: get_color_values
 * --------------------
 * Read the color sensor in RGB mode and extract the three values
 * --------------------
 * color_val: pointer to the struct to hold the RGB values
 * color: handle of the color sensor
 * --------------------
 * Made by Martina
 */
void get_color_values(rgb * color_val, uint8_t color){
 	get_sensor_value0(color, &(color_val->r));
 	get_sensor_value1(color, &(color_val->g));
 	get_sensor_value2(color, &(color_val->b));
}

/*
 * Function: get_main_color
 * --------------------
 * Given the RGB values read from the color sensor, find the main color.
 * The main color is valid only if a certain threshold is satisfied.
 * Also, particular care in identifying RED is taken.
 * --------------------
 * color_val: pointer to the struct to hold the RGB values
 * main_color: buffer where to write the main color
 * --------------------
 * return: whether the main color is valid or not
 * --------------------
 * Made by Martina
 */
int get_main_color(rgb * color_val, char * main_color){
  int valid;
  if (color_val->r < 3 && color_val->g < 3 && color_val->b < 3){
  	valid = 0;
  } else {
  	valid = 1;
  }
	float main_color_val = 5;
	strcpy(main_color, "BLACK");
	if(color_val->r > 5 && color_val->r > 3*color_val->g && color_val->r > 3*color_val->b){
  	main_color_val = color_val->r;
  	strcpy(main_color, "RED");
	}
	main_color_val = 20;
  if(color_val->g > main_color_val){
  	main_color_val = color_val->g;
  	strcpy(main_color, "GREEN");
  }
  if(color_val->b > main_color_val){
  	main_color_val = color_val->b;
  	strcpy(main_color, "BLUE");
  }
  return valid;
}

/*
 * Function: get_color
 * --------------------
 * Wrapper for reading the color sensor and finding the main color together.
 * --------------------
 * color: handle of the color sensor
 * buf: buffer where to write the main color
 * --------------------
 * return: whether the main color is valid or not
 * --------------------
 * Made by Martina
 */
int get_color(uint8_t color, char * buf){
  rgb color_val;
	get_color_values(&color_val, color);
	return get_main_color(&color_val, buf);
}

/*
 * Function: front_obstacle
 * --------------------
 * Function to read the ultrasonic sensor and interpret the result.
 * --------------------
 * dist: handle of the distance sensor
 * --------------------
 * return: 0 if no object is seen, or if the distance is higher than a certain confidence threshold.
 * --------------------
 * Made by Luca
 */
int front_obstacle(uint8_t dist) {
	  int distance;
		if ((distance = (int)get_value_samples(dist, 3)) <= DIST_THRESHOLD )
			return distance;
		else
			return 0;
}

/*
 * Function: scan_around
 * --------------------
 * Thread that movers the ultrasonic sensor left and right to better see surroundings.
 * --------------------
 * arg: handle of the distance sensor
 * --------------------
 * Made by Paolo
 */
void * scan_around(void * arg) {
	uint8_t motor_head = * (uint8_t *) arg;
	for (;;) {
		turn_motor_to_pos(motor_head, MAX_SPEED/4, -45);
		wait_motor_stop(motor_head);
		turn_motor_to_pos(motor_head, MAX_SPEED/4, 0);
		wait_motor_stop(motor_head);
		turn_motor_to_pos(motor_head, MAX_SPEED/4, 45);
		wait_motor_stop(motor_head);
		turn_motor_to_pos(motor_head, MAX_SPEED/4, 0);
		wait_motor_stop(motor_head);
	}
}

/*
 * Function: go_forwards_obs
 * --------------------
 * Moves the robot ahead until an obstacle is encountered.
 * Conditions to stop are multiple:
 *  - the obstacle is seen by the rotating head
 *  - the obstacle bumps into the touch sensor
 *  - the robot would cross the virtual wall at the back of the arena
 * --------------------
 * motors: handle of the motor array to control
 * motor_head: handle of the head motor to control
 * dist: handle of the distance sensor
 * touch: handle of the touch sensor
 * cm: maximum distance of an obstacle before stopping
 * speed: velocity to turn the motors at
 * --------------------
 * return: the direction of the ultrasonic sensor where the obstacle was seen.
 * --------------------
 * Made by Valerio
 */
int go_forwards_obs(uint8_t *motors, uint8_t motor_head ,uint8_t dist, uint8_t touch, int cm, int speed) {
	int contact, distance, head_pos, ret = 0;
	int th = 5;
	pthread_t scanner;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], MOT_DIR * speed * COMP_SX);
	set_tacho_speed_sp( motors[1], MOT_DIR * speed * COMP_DX);
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
	//Spawn a thread to continuously turn the head arounf
	pthread_create( &scanner, NULL, scan_around, (void *)&motor_head);
	do {
		//Read the ultrasonic sensor
		distance = front_obstacle(dist);
		//Read the head position, for later
		get_tacho_position(motor_head, &head_pos);
		//Read the touch sensor
		contact = get_value_single(touch);
	} while ((distance == 0 || distance > cm*10.0) && contact == 0 && my_pos.y > th+P); //If any of the aforementioned conditions is satisfied, stop
	if (contact == 0) {
		//If we didn't stop because of bumping, we want to know the head position
		ret = head_pos;
	}
	//Stop the motors, cancel the thread, reset the head
	stop_motors(motors);
	pthread_cancel(scanner);
	turn_motor_to_pos(motor_head, speed, 0);
	wait_motor_stop(motor_head);
	return ret;
}

/*
 * Function: check_ball
 * --------------------
 * Checks if the robot has a red ball in fornt of it and takes appropriate action.
 * --------------------
 * dist: handle of the distance sensor
 * color: handle of the color sensor
 * angle: angle which the robot is looking at
 * --------------------
 * return: whether the ball was detected or not
 * --------------------
 * Made by Martina
 */
int check_ball(uint8_t dist, uint8_t color, int angle) {
	char s[10];
 	int d = front_obstacle(dist);
 	float x = my_pos.x, y = my_pos.y;
 	get_color(color, s);
	float dx = d/10.0 + FACE;
	float xb = x + dx * sin((angle * M_PI) / 180.0);
	float yb = y + dx * cos((angle * M_PI) / 180.0);

 	if (d > 0 && strcmp(s, "RED") == 0) {
		printf("----------   Movable obstacle found at %5.1f, %5.1f   ----------\n", xb-P, yb-P);
		map_fix(xb, yb, angle, 5, 5, SURE_MISS);
		return 1;
 	} else if (d > 0) {
		printf("---------- Non movable obstacle found at %5.1f, %5.1f ----------\n", xb-P, yb-P);
	}
	return 0;
}

/*
 * Function: scan_for_obstacle_N_pos
 * --------------------
 * Makes the robot turn around to detect surroundings.
 * The whole robot turns with the wheels, trusting the gyroscope.
 * --------------------
 * motors: handle of the motor array to control
 * dist: handle of the distance sensor
 * gyro: handle of the gyroscope module
 * obstacles: array of distances, will be populated by the function
 * angles: array of angles at which the obsdtacles were seen, will be populated by the function
 * pos: number of distinct positions to take measurements at
 * span: total angle to span while taking measurements
 * final_dir: final direction of the robot. Allows for slight optimization so we avoid turning once finished.
 * speed: velocity to turn the motors at
 * --------------------
 * Made by Luca
 */
void scan_for_obstacle_N_pos(uint8_t *motors, uint8_t dist, uint8_t gyro, int* obstacles, int* angles, int pos, int span, int final_dir, int speed) {
	 int dir, d;
	 int angle, i;
	 float anglef;
	 dir = (int)get_value_single(gyro) % 360;
	 anglef = (float)(span) / (pos-1);
	 angle = (int) (anglef);
	 d = front_obstacle(dist);
	 obstacles[(pos-1)/2] = d;
	 angles[(pos-1)/2] = 0;
	 if (final_dir == 1) {
	  for (i=0;i<((pos-1)/2);i++) {
		  turn_left_gyro (motors, gyro, speed, angle);
		  d = front_obstacle(dist);
			obstacles[((pos-1)/2)-(i+1)] = d;
		  angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
	  }
		turn_right_gyro(motors, gyro, MAX_SPEED/16, span/2);
	  for (i=(((pos-1)/2)+1);i<pos;i++) {
		  turn_right_gyro (motors, gyro, speed, angle);
			d = front_obstacle(dist);
		  obstacles[i] = d;
		  angles[i] = (int)((i)*anglef-(span/2));
	  }
	 }
	 if (final_dir == -1) {
		 for (i=(((pos-1)/2)+1);i<pos;i++) {
			 turn_right_gyro (motors, gyro, speed, angle);
			 d = front_obstacle(dist);
			 obstacles[i] = d;
			 angles[i] = (int)((i)*anglef-(span/2));
		 }
		 turn_left_gyro(motors, gyro, MAX_SPEED/16, span/2);
		 for (i=0;i<((pos-1)/2);i++) {
			 turn_left_gyro (motors, gyro, speed, angle);
			 d = front_obstacle(dist);
			 obstacles[((pos-1)/2)-(i+1)] = d;
			 angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
		 }
	 }
	 if (final_dir == 0) {
		 for (i=0;i<((pos-1)/2);i++) {
			 turn_left_gyro (motors, gyro, speed, angle);
			 d =
			 obstacles[((pos-1)/2)-(i+1)] = front_obstacle(dist);
			 angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
		 }
		 turn_right_gyro(motors, gyro, MAX_SPEED/16, span/2);
		 for (i=(((pos-1)/2)+1);i<pos;i++) {
			 turn_right_gyro (motors, gyro, speed, angle);
			 d = front_obstacle(dist);
			 obstacles[i] = d;
			 angles[i] = (int)((i)*anglef-(span/2));
		 }
		 turn_to_angle (motors, MAX_SPEED/16, dir);
	 }
 }

 /*
  * Function: scan_for_obstacle_N_pos_head
  * --------------------
  * Makes the robot scan around to detect surroundings.
  * Only the head with the ultrasonic sensor will turn around with a specific motor.
  * --------------------
  * motors: handle of the motor array to control
  * dist: handle of the distance sensor
  * obstacles: array of distances, will be populated by the function
  * angles: array of angles at which the obsdtacles were seen, will be populated by the function
  * pos: number of distinct positions to take measurements at
  * span: total angle to span while taking measurements
  * speed: velocity to turn the motors at
  * --------------------
  * Made by Luca
  */
 void scan_for_obstacle_N_pos_head(uint8_t motor, uint8_t dist, int* obstacles, int* angles, int pos, int span, int speed) {
	 int i;
	 float anglef = (float)(span) / (pos-1);
	 turn_motor_to_pos(motor, speed, 0);
	 wait_motor_stop(motor);
	 obstacles[(pos-1)/2] = front_obstacle(dist);
	 angles[(pos-1)/2] = 0;
	 for (i=0;i<((pos-1)/2);i++) {
		 turn_motor_to_pos(motor, speed, (i+1)*anglef);
		 wait_motor_stop(motor);
		 obstacles[((pos-1)/2)-(i+1)] = front_obstacle(dist);
		 angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
	 }
	 turn_motor_to_pos(motor, speed, 0);
	 wait_motor_stop(motor);
	 for (i=(((pos-1)/2)+1);i<pos;i++) {
		 turn_motor_to_pos(motor, speed, (-1*i)*anglef+(span/2));
		 wait_motor_stop(motor);
		 obstacles[i] = front_obstacle(dist);
		 angles[i] = (int)((i)*anglef-(span/2));
	 }
	 turn_motor_to_pos(motor, speed, 0);
	 wait_motor_stop(motor);
 }

 /*
  * Function: turn_motor_obs_to_pos_down
  * --------------------
  * Turn the obstacle motor down to release obstacle.
  * --------------------
  * motor: handle of the motor to turn
	* speed: velocity to turn the motor at
	* pos: amount of degrees to turn the motor by
  * --------------------
  * Made by Martina
  */
void turn_motor_obs_to_pos_down(uint8_t motor, int speed, float pos){
	set_tacho_stop_action_inx( motor, STOP_ACTION );
 	set_tacho_speed_sp( motor, speed *  MOT_DIR );
 	set_tacho_ramp_up_sp( motor, 0 );
 	set_tacho_ramp_down_sp( motor, 0 );
 	set_tacho_position_sp( motor, pos );
 	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
 }

 /*
  * Function: turn_motor_obs_to_pos_up
  * --------------------
  * Turn the obstacle motor up to hold an object or
	* return to initial position
  * --------------------
  * motor: handle of the motor to turn
	* speed: velocity to turn the motor at
	* pos: amount of degrees to turn the motor by
  * --------------------
  * Made by Martina
  */
 void turn_motor_obs_to_pos_up(uint8_t motor, int speed, float pos){
  set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed *  MOT_DIR );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	set_tacho_position_sp( motor, pos );
	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
}

/*
 * Function: angle_recal
 * --------------------
 * Scan the surroundings turning the head by small angles
 * and acquiring a vector of distances.
 * Turn to the angle of minimum distance.
 * --------------------
* motors: handle of the motor array to control
* head: handle of the head motor to control
* dist: handle of the distance sensor
* gyro: handle of the gyroscope module
* samples: number of distinct positions to take measurements at
* deg: amount of degree between one sample and the other one
* th: threashold at which the calibration is executed
* gyro_lock: mutex to access gyro history
* —------------------
* Made by Paolo
*/
void angle_recal(uint8_t *motors, uint8_t head, uint8_t dist, uint8_t gyro, int samples, int deg, int th, pthread_mutex_t *gyro_lock) {
	int dir = my_pos.dir;
	//If we aren't drifted by at least th degrees, just exit
	if (abs(dir) > th) {
		return;
	}
	int cnt = 0, best = 0;
	int i;
	int d, min = 3000.0;
  int pos = ((samples-1)/2)*deg*-1;
	//Scan the different positions
	for (i = 0; i < samples; i++) {
		turn_motor_to_pos(head, MAX_SPEED/32, -pos);
		wait_motor_stop(head);
		d = front_obstacle(dist);
		//If this smallest the closest measurement so far, we are closest to the wall
		if (d < min && d != 0) {
			min = d;
			best = pos;
			cnt = 1;
		}
		else if (d == min) {
			//Count how many ties we have
			cnt++;
		}
		pos += deg;
	}
	//Find the correct angle to turn at, accounting for ties
	best = (best*2 + (cnt-1)*deg)/2;
	turn_motor_to_pos(head, MAX_SPEED/32, 0);
	wait_motor_stop(head);
	if (cnt == 0 ){
	 	return;
	}
	if (best > 0) {
		//Turn right to recalibrate
		turn_right(motors, MAX_SPEED/16, best);
		wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
		my_pos.dir = 0;
	} else {
		//Turn right to recalibrate
		turn_right(motors, MAX_SPEED/16, best);
		wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
		my_pos.dir = 0;
	}
	//Reset the gyroscope to correct it with the new position
	millisleep(50);
	pthread_mutex_lock(gyro_lock);
	set_gyro(gyro);
	pthread_mutex_unlock(gyro_lock);
	millisleep(50);
}

/*
 * Function: release_obs_routine
 * —------------------
 * Thread to release the obstacle. Lower the arms,
 * map the obstacle, go forward by 5cm, raise the arms
 * —------------------
 * thread_args composed of:
 * - motor: handle of the obstacle motor
 * - motor0 and motor1: handles of the two motor to control
 * - speed: velocity to turn the motors at
 * - pos_down: angle to lower the arms at
 * - pos_up: angle to raise the arms at
 * —------------------
 * Made by Martina
 */
void * release_obs_routine(void * thread_args){
	struct obstacle_thread_arguments * args;
	args = (struct obstacle_thread_arguments *) thread_args;
	int motor = args->motor;
	uint8_t motors[2];
	motors[0] = args->motor0;
	motors[1] = args->motor1;
	int speed = args->speed;
	int pos_down = args->pos_down;
	int pos_up = args->pos_up;

	int x = (int)my_pos.x, y = (int)my_pos.y; int dir = (int)my_pos.dir;
	turn_motor_obs_to_pos_down(motor, speed, pos_down);
	wait_motor_stop(motor);
	//Maths to compute the coordinates of the obstacle based on the robot position and size
	int obs_p1 = x-(TAIL+SIDEY_OBSTACLE)*(sin(dir))-SIDEX_OBSTACLE/2*(cos(dir));
  int obs_p2 = y-(TAIL+SIDEY_OBSTACLE)*(cos(dir))-SIDEX_OBSTACLE/2*(sin(dir));
  int obs_p3 = x - TAIL*(sin(dir)) + SIDEX_OBSTACLE/2*(cos(dir));
  int obs_p4 = y - SIDEX_OBSTACLE*(sin(dir)) - TAIL*(cos(dir));
	//Add the obstacle to the map
  add_my_obstacle(obs_p1, obs_p2, obs_p3, obs_p4);
	go_forwards_cm(motors, 5, speed*2);
	turn_motor_obs_to_pos_up(motor, speed, pos_up);
	wait_motor_stop(motor);
	return NULL;
}

/*
  * Function: go_forwards_cm_obs
  * —------------------
  * Makes the robot go forward either by a specified
	* number of cm or until it finds an obstacle.
  * —------------------
  * motors: handle of the motor array to control
	* motor_head: handle of the head motor to control
  * dist: handle of the distance sensor
  * touch: handle of the touch sensor
  * cm: number of cm to move forward by
	* max_dist: maximum distance of an obstacle before stopping
  * speed: velocity to turn the motors at
  * —------------------
	* return: 1 if the robot completes the movement, 0 if it
	* found an obstacle before
	* —------------------
  * Made by Valerio
  */
int go_forwards_cm_obs(uint8_t *motors, uint8_t motor_head, uint8_t dist, uint8_t touch, int cm, int max_dist, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	FLAGS_T state0, state1;
	int contact, distance, ret = 0;
	pthread_t scanner;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], speed * COMP_SX);
	set_tacho_speed_sp( motors[1], speed * COMP_DX);
	multi_set_tacho_position_sp( motors, MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	//Spawn a thread to continuously turn the head arounf
	pthread_create( &scanner, NULL, scan_around, (void *)&motor_head);

	do {
		get_tacho_state_flags( motors[0], &state0 );
		get_tacho_state_flags( motors[1], &state1 );
		//Read the ultrasonic sensor
		distance = front_obstacle(dist);
		//Read the touch sensor
		contact = get_value_single(touch);
	} while ((distance == 0 || distance > max_dist*10.0) && contact == 0 && state0 && state1);

	if (state0 || state1) {
		//If we successfully travelled for all the cm, we want to know
		ret = 1;
	}
	//Stop the motors, cancel the thread, reset the head
	stop_motors(motors);
	pthread_cancel(scanner);
	turn_motor_to_pos(motor_head, speed, 0);
	wait_motor_stop(motor_head);
	return ret;
}

/*
  * Function: panic
  * —------------------
  * Check whether the robot has drifted from the expected position
	* and correct it
  * —------------------
  * motors: handle of the motor array to control
	* pos_lock: mutex to safely access the position structure
  * —------------------
	* return: whether it has panicked or not
	* —------------------
  * Made by Paolo
  */
int panic(uint8_t *motors, pthread_mutex_t *pos_lock)
{
	int th = 15, panicked = 0;
	printf("%f %f\n",my_pos.dir, gyro_pos.dir);

	//Check if we think we are looking at 0 but drifted by more than th degrees
	if (my_pos.dir == 0.0 && abs(gyro_pos.dir)>th) {
		turn_to_angle(motors, MAX_SPEED/16, 0);
		panicked = 1;
	}
	//Check if we think we are looking at 90 but drifted by more than th degrees
	else if (my_pos.dir == 90.0 && abs(gyro_pos.dir-90)>th) {
		turn_to_angle(motors, MAX_SPEED/16, 90);
		panicked = 1;
	}
	//Check if we think we are looking at 180 but drifted by more than th degrees
	else if (my_pos.dir == 180.0 && abs(gyro_pos.dir)-180<-th) {
		turn_to_angle(motors, MAX_SPEED/16, 180);
		panicked = 1;
	}
	//Check if we think we are looking at -90 but drifted by more than th degrees
	else if (my_pos.dir == -90.0 && abs(gyro_pos.dir+90)>th) {
		turn_to_angle(motors, MAX_SPEED/16, -90);
		panicked = 1;
	}

	if (panicked) {
		//If we had to recalibrate, update position
		pthread_mutex_lock(pos_lock);
		my_pos.x = gyro_pos.x;
		my_pos.y = gyro_pos.y;
		pthread_mutex_unlock(pos_lock);
	}

	return panicked;
}
