#define millisleep( msec ) usleep(( msec ) * 1000 )

typedef struct position {
	int x;
	int y;
	int dir;
} position;

void turn_motor_time(uint8_t sn, int speed, int time, int ramp_up, int ramp_down) {
	set_tacho_stop_action_inx( sn, TACHO_COAST );
	set_tacho_speed_sp( sn, speed );
	set_tacho_time_sp( sn, time );
	set_tacho_ramp_up_sp( sn, ramp_up );
	set_tacho_ramp_down_sp( sn, ramp_down );
	set_tacho_command_inx( sn, TACHO_RUN_TIMED );
}

void turn_motor_deg(uint8_t sn, int speed, int deg) {
	int compensation = 1;
	set_tacho_stop_action_inx( sn, TACHO_COAST );
	set_tacho_speed_sp( sn, speed * compensation );
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_position_sp( sn, deg );
	set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
}

void turn_motor_to_pos(uint8_t sn, int speed, int pos) {
	int compensation = 1;
	set_tacho_stop_action_inx( sn, TACHO_COAST );
	set_tacho_speed_sp( sn, speed * compensation );
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_position_sp( sn, pos );
	set_tacho_command_inx( sn, TACHO_RUN_TO_ABS_POS );
}

void go_forwards(uint8_t *sn, int time, int speed) {
	multi_set_tacho_stop_action_inx( sn, TACHO_COAST );
	multi_set_tacho_speed_sp( sn, -speed );
	multi_set_tacho_time_sp( sn, time );
	multi_set_tacho_ramp_up_sp( sn, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( sn, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( sn, TACHO_RUN_TIMED );
	float extimation = M_PI;
}

void turn_right(uint8_t *sn, int speed, int deg) {
	int compensation = 1;
	multi_set_tacho_stop_action_inx( sn, TACHO_COAST );
	multi_set_tacho_speed_sp( sn, speed * compensation );
	multi_set_tacho_ramp_up_sp( sn, 0 );
	multi_set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_position_sp( sn[0], -(TURN360*360)/deg );
	set_tacho_position_sp( sn[1], (TURN360*360)/deg );
	multi_set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
}

void turn_left(uint8_t *sn, int speed, int deg) {
	int compensation = 1;
	multi_set_tacho_stop_action_inx( sn, TACHO_COAST );
	multi_set_tacho_speed_sp( sn, speed * compensation );
	multi_set_tacho_ramp_up_sp( sn, 500 );
	multi_set_tacho_ramp_down_sp( sn, 500 );
	set_tacho_position_sp( sn[0], (TURN360*360)/deg);
	set_tacho_position_sp( sn[1], -(TURN360*360)/deg );
	multi_set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
}

void wait_motor_stop(uint8_t sn) {
	FLAGS_T state;
	do {
		get_tacho_state_flags( sn, &state );
	} while ( state );
}

float get_value_single(uint8_t sn) {
	float val;
	get_sensor_value0( sn, &val );
	return val;
}

float get_value_samples(uint8_t sn, int samples) {
	float val, sum = 0;
	int i;
	for (i = 0; i < samples; i++) {
		get_sensor_value0( sn, &val );
		sum  += val;
	}
	return sum/samples;
}
