#include <string.h>
#define millisleep( msec ) usleep(( msec ) * 1000 )

const char const *colors[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( colors ) / sizeof( colors[ 0 ])))

typedef struct position {
	int x;
	int y;
	int dir;
} position;

position my_pos = { .x = START_X, .y = START_Y, .dir = START_DIR };

// void update_direction(int ) {
//
// }

void update_position(int dist) {  // TODO test
	my_pos.x += dist * sin((my_pos.dir * M_PI) / 180.0);
	my_pos.y += dist * cos((my_pos.dir * M_PI) / 180.0);
}

typedef struct rgb {
 	float r;
 	float g;
 	float b;
 } rgb;


void turn_motor_time(uint8_t motor, int speed, int time, int ramp_up, int ramp_down) {
	set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed );
	set_tacho_time_sp( motor, time );
	set_tacho_ramp_up_sp( motor, ramp_up );
	set_tacho_ramp_down_sp( motor, ramp_down );
	set_tacho_command_inx( motor, TACHO_RUN_TIMED );
}

void turn_motor_deg(uint8_t motor, int speed, int deg) {
	set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	set_tacho_position_sp( motor, deg );
	set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}

void turn_motor_to_pos(uint8_t motor, int speed, int pos) {
	set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed *  MOT_DIR );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	set_tacho_position_sp( motor, pos );
	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
}

void go_forwards_time(uint8_t *motors, int time, int speed) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, MOT_DIR * speed );
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	// TODO valerio
	//update_position(int DA_CALCOLARE)
}

void go_backwards_time(uint8_t *motors, int time, int speed) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, -MOT_DIR * speed );
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	// TODO valerio
	//update_position(int DA_CALCOLARE*-1);
}

void go_forwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	//printf("%f\n", deg);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_position_sp( motors, MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	update_position(cm);
}

void go_backwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	//printf("%f\n", deg);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_position_sp( motors, -MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	update_position(cm * -1);
}

void turn_right(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	set_tacho_position_sp( motors[0], MOT_DIR*(TURN360*deg)/360 );
	set_tacho_position_sp( motors[1], -MOT_DIR*(TURN360*deg)/360 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
}

void turn_left(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	set_tacho_position_sp( motors[0], - MOT_DIR*(TURN360*deg)/360);
	set_tacho_position_sp( motors[1], MOT_DIR*(TURN360*deg)/360 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
}

void wait_motor_stop(uint8_t motor) { // sometimes don't work properly, TODO fix
	FLAGS_T state;
	do {
		get_tacho_state_flags( motor, &state );
	} while ( state );
}

void stop_motors(uint8_t *motors) {
	multi_set_tacho_command_inx( motors, TACHO_STOP );
}

float get_value_single(uint8_t sensor) {
	float val;
	get_sensor_value0( sensor, &val );
	return val;
}

float get_value_samples(uint8_t sensor, int samples) {
	float val, sum = 0;
	int i;
	for (i = 0; i < samples; i++) {
		get_sensor_value0( sensor, &val );
		sum  += val;
	}
	return sum/samples;
}

float get_compass_value_samples(uint8_t compass, int samples) { // problem when samples cross the 0-360 boundary, TODO solve
	float val, sum = 0;
	int i;
	for (i = 0; i < samples; i++) {
		get_sensor_value0( compass, &val );
		sum  += val;
	}
	float r = sum/samples;
	return r > 180 ? r - 360 : r;
}




void turn_right_compass(uint8_t *motors, uint8_t compass, int speed, int deg) { // TODO check for values < 0 or > 180
	int dir, start_dir = get_compass_value_samples( compass, 5 );
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
	// printf("%d\n", start_dir);
	if (start_dir + deg > 180) {
		int end_dir = start_dir - 360 + deg;
		do {
	    dir = get_compass_value_samples( compass, 5 ); // TODO fix precision
		} while (dir > 0 || dir < end_dir);
		stop_motors(motors);
	} else {
		int end_dir = start_dir + deg;
		do {
	    dir = get_compass_value_samples( compass, 5 ); // TODO fix precision
		} while (dir < end_dir);
		stop_motors(motors);
	}
	// printf("%d\n", dir);
}

void turn_left_compass(uint8_t *motors, uint8_t compass, int speed, int deg) { // TODO check for values < 0 or > 180
	int dir, start_dir = get_compass_value_samples( compass, 5 );
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	set_tacho_speed_sp( motors[1], MOT_DIR * speed);
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
  // printf("%d\n", start_dir);
	if (start_dir - deg < -180) {
		int end_dir = start_dir + 360 - deg;
		do {
	    dir = get_compass_value_samples( compass, 5 ); // TODO fix precision
		} while (dir < 0 || dir > end_dir);
		stop_motors(motors);
	} else {
		int end_dir = start_dir - deg;
		do {
	    dir = get_compass_value_samples( compass, 5 ); // TODO fix precision
		} while (dir > end_dir);
		stop_motors(motors);
	}
	// printf("%d\n", dir);
}

void turn_to_angle(uint8_t *motors, uint8_t gyro, int speed, int deg) {
 	int start_pos = (int)get_value_single(gyro);
 	//printf("start_pos=%d\n", start_pos);
 	int cur_pos, dest;
	int a;
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );

	a = ((start_pos % 360) + 360) % 360;
	if (deg > a) {
		if ((deg - a) < 180 ) {
			// turn clockwise
			set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
			dest = start_pos - a + deg;
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
		 	do {
		 		cur_pos = (int)get_value_single(gyro);
		 		//printf("cur_pos_reinitializing=%d\n", cur_pos);
		 	} while(cur_pos <= dest);
		 	stop_motors(motors);
		}
		else {
			// turn counterclockwise
			set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
			dest = start_pos - a + deg - 360;
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
		 	do {
		 		cur_pos = (int)get_value_single(gyro);
		 		//printf("cur_pos_reinitializing=%d\n", cur_pos);
		 	} while(cur_pos >= dest);
		 	stop_motors(motors);
		}
	}
	else {
		if ((a - deg) < 180) {
			// turn counterclockwise
			set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
			dest = start_pos - a + deg;
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
		 	do {
		 		cur_pos = (int)get_value_single(gyro);
		 		//printf("cur_pos_reinitializing=%d\n", cur_pos);
		 	} while(cur_pos >= dest);
		 	stop_motors(motors);
		}
		else {
			// turn clockwise
			set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
			dest = start_pos - a + deg + 360;
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
		 	do {
		 		cur_pos = (int)get_value_single(gyro);
		 		//printf("cur_pos_reinitializing=%d\n", cur_pos);
		 	} while(cur_pos <= dest);
		 	stop_motors(motors);
		}
	}

 }

 void reinit_pos_gyro(uint8_t *motors, uint8_t gyro, int speed) {
  	float start_pos = get_value_samples(gyro,5);
  	printf("start_pos=%f\n", start_pos);
  	float cur_pos;
  	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
  	multi_set_tacho_ramp_up_sp( motors, 0 );
  	multi_set_tacho_ramp_down_sp( motors, 0 );

  	if(start_pos < 0){
  		set_tacho_speed_sp( motors[0], MOT_DIR * speed);
  		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
  	} else {
  		set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
  		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
  	}

  	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
  	do {
  		cur_pos = get_value_samples(gyro, 5);
  		//printf("cur_pos_reinitializing=%f\n", cur_pos);
  	} while(cur_pos < -1 || cur_pos > 1);
  	stop_motors(motors);

  }

	void init_gyro(uint8_t *motors, uint8_t gyro, int speed){
		turn_to_angle(motors, gyro, speed, 0);
		set_sensor_mode_inx(gyro, GYRO_GYRO_RATE);
		set_sensor_mode_inx(gyro, GYRO_GYRO_ANG);
	}

	void set_gyro(uint8_t gyro){
			set_sensor_mode_inx(gyro, GYRO_GYRO_RATE);
			set_sensor_mode_inx(gyro, GYRO_GYRO_ANG);
	}


 void turn_left_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg) {
 	float start_dir = get_value_samples( gyro, 5 );
 	float dir;
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
 	set_tacho_speed_sp( motors[1], MOT_DIR * speed);
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );
 	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
 	float end_dir = start_dir-deg;
 	do {
 			dir = get_value_single(gyro);
 			printf("cur_pos=%f\n", dir);
 		} while (dir >= end_dir);
 		stop_motors(motors);
 }

 void turn_right_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg) {
 	float start_dir = get_value_samples( gyro, 5 );
 	float dir;
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	set_tacho_speed_sp( motors[0], MOT_DIR * speed);
 	set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );
 	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
 	float end_dir = start_dir+deg;
 	do {
 			dir = get_value_single(gyro);
 			//printf("cur_pos=%f\n", dir);
 		} while (dir <= end_dir);
 		stop_motors(motors);
 }

 void get_color_values(rgb * color_val, uint8_t color){
 	get_sensor_value0(color, &(color_val->r));
 	get_sensor_value1(color, &(color_val->g));
 	get_sensor_value2(color, &(color_val->b));

 }

 int get_main_color(rgb * color_val, char * main_color){
  	 int valid;
    if (color_val->r < 3 && color_val->g < 3 && color_val->b < 3){
  		valid = 0;
  	} else {
  		valid = 1;
  	}
  	float main_color_val = color_val->r;
  	strcpy(main_color, "RED");
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


int front_obstacle (uint8_t dist) {
	  int distance;
		if ((distance = (int)get_value_single(dist)) <= DIST_THRESHOLD )
			return distance;
		else
			return 0;
}

void go_forwards_obs(uint8_t *motors, uint8_t dist, int cm, int speed) {
	float d;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, MOT_DIR * speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
	do {
		d = front_obstacle(dist);
		// printf("%f\n", d);
	} while (d == 0 || d > cm*10);
	stop_motors(motors);
	// TODO valerio
	//update_position(int DA_CALCOLARE)
}


 void scan_for_obstacle_N_pos (uint8_t *motors, uint8_t dist, uint8_t gyro, int* obstacle, int pos, int span) {
	 int dir;
	 int angle, i;
	 dir = (int)get_value_single(gyro) % 360;
	 printf("initial direction %d\n", dir);
	 angle = (span) / ((pos-1));
	 obstacle[(pos-1)/2] = front_obstacle(dist);
	 for (i=0;i<((pos-1)/2);i++) {
		 turn_left_gyro (motors, gyro, MAX_SPEED/16, angle);
		 obstacle[((pos-1)/2)-i] = front_obstacle(dist);
	 }
	 turn_to_angle (motors, gyro, MAX_SPEED/16, dir);
	 for (i=(((pos-1)/2)+1);i<pos;i++) {
		 turn_right_gyro (motors, gyro, MAX_SPEED/16, angle);
		 obstacle[i] = front_obstacle(dist);
	 }
	 turn_to_angle(motors, gyro, MAX_SPEED/16, dir);
 }

 void scan_for_obstacle_N_pos_head (uint8_t motors, uint8_t dist, int* obstacle, int pos, int span) {
	 int angle, i;
	 angle = (span) / ((pos-1));
	 obstacle[(pos-1)/2] = front_obstacle(dist);
	 for (i=0;i<((pos-1)/2);i++) {
		 turn_motor_deg(motors, MAX_SPEED/16, angle);
		 wait_motor_stop(motors);
		 obstacle[((pos-1)/2)-i] = front_obstacle(dist);
	 }
	 turn_motor_deg(motors, MAX_SPEED/16, ((-1)*(span/2)));
	 wait_motor_stop(motors);
	 for (i=(((pos-1)/2)+1);i<pos;i++) {
		 turn_motor_deg(motors, MAX_SPEED/16, ((-1)*angle));
		 wait_motor_stop(motors);
		 obstacle[i] = front_obstacle(dist);
	 }
	 turn_motor_deg(motors, MAX_SPEED/16, (span/2));
	 wait_motor_stop(motors);
 }
