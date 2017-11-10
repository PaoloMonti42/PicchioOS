#define millisleep( msec ) usleep(( msec ) * 1000 )

typedef struct position {
	int x;
	int y;
	int dir;
} position;

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
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	//float extimation = (M_PI*WHEEL_DIAM*deg)/360.0; TODO
}

void go_backwards_time(uint8_t *motors, int time, int speed) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, -MOT_DIR * speed );
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	//float extimation = (M_PI*WHEEL_DIAM*deg)/360.0; TODO
}

void go_forwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	printf("%f\n", deg);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_position_sp( motors, MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
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
	multi_set_tacho_ramp_up_sp( motors, 500 );
	multi_set_tacho_ramp_down_sp( motors, 500 );
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

void update_direction(int *direction, int direction_offset, uint8_t compass, int samples) {
	int current = get_compass_value_samples( compass, samples );
	int dir = current - direction_offset + START_DIR;
	*direction = dir > 180 ? dir - 360 : (dir < -180 ? dir + 360 : dir);
}

void update_position(position *pos, int dist) {  // TODO test
	pos->x += dist * sin((pos->dir * M_PI) / 180.0);
	pos->y += dist * cos((pos->dir * M_PI) / 180.0);
}

void go_forwards_obs(uint8_t *motors, uint8_t dist, int cm, int speed) {
	float d;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, MOT_DIR * speed );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
	do {
    d = get_value_samples( dist, 2 ); // TODO fix precision
		// printf("%f\n", d);
	} while (d > cm*10);
	stop_motors(motors);
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

int front_obstacle () {
	  int distance;
		if ((distance = (int)get_value_single(dist)) <= 110 )
			return distance;
		else
			return 0;
}

 void scan_for_obstacle_5_pos (uint8_t *motors,int* obstacle) {
	 int dir;
	 dir = get_compass_value_samples( compass, 5 );
	 obstacle[2] = front_obstacle();
	 turn_left_compass (motors, compass, MAX_SPEED/10, 45);
	 obstacle[1] = front_obstacle();
	 turn_left_compass (motors, compass, MAX_SPEED/10, 45);
	 obstacle[0] = front_obstacle();
	 turn_right_compass (motors, compass, MAX_SPEED/10, 90);
	 turn_right_compass (motors, compass, MAX_SPEED/10, 45);
	 obstacle[3] = front_obstacle();
	 turn_right_compass (motors, compass, MAX_SPEED/10, 45);
	 obstacle[4] = front_obstacle();
	 turn_left_compass (motors, compass, MAX_SPEED/10, 90);
 }
