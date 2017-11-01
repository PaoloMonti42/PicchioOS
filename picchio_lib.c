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

void go_forwards(uint8_t *motors, int time, int speed) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, MOT_DIR * speed );
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	float extimation = (M_PI*WHEEL_DIAM);
}

void go_backwards(uint8_t *motors, int time, int speed) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, -MOT_DIR * speed );
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	float extimation = (M_PI*WHEEL_DIAM);
}

void turn_right(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	set_tacho_position_sp( motors[0], MOT_DIR*(TURN360*360)/deg );
	set_tacho_position_sp( motors[1], -MOT_DIR*(TURN360*360)/deg );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
}

void turn_left(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 500 );
	multi_set_tacho_ramp_down_sp( motors, 500 );
	set_tacho_position_sp( motors[0], - MOT_DIR*(TURN360*360)/deg);
	set_tacho_position_sp( motors[1], MOT_DIR*(TURN360*360)/deg );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
}

void wait_motor_stop(uint8_t motor) {
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
		sum  += val; //TODO compass on the edge
	}
	return sum/samples;
}

float get_compass_value_samples(uint8_t compass, int samples) {
	float val, sum = 0;
	int i;
	for (i = 0; i < samples; i++) {
		get_sensor_value0( compass, &val );
		sum  += val;
	}
	float r = sum/samples;				// TODO fix when averaging on the edge of 360 and 0
	return r > 180 ? r - 360 : r; // HACK -180 to 180. Fixes todo above?  
}

void update_direction(int *direction, int direction_offset, uint8_t compass, int samples) {
	int current = get_compass_value_samples( compass, samples );
	int dir = current - direction_offset + START_DIR;
	*direction = dir > 180 ? dir - 360 : (dir < -180 ? dir + 360 : dir);
}

void turn_right_compass(uint8_t *motors, uint8_t compass, int speed, int deg) {
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

void turn_left_compass(uint8_t *motors, uint8_t compass, int speed, int deg) {
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
