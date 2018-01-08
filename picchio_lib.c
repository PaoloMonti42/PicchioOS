#include <string.h>
#include <math.h>

#define millisleep( msec ) usleep(( msec ) * 1000 )
#include <sys/timeb.h>
//#include <math.h>
const char const *colors[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( colors ) / sizeof( colors[ 0 ])))

typedef struct position {
	volatile float x;
	volatile float y;
	volatile float dir;
} position;

position my_pos;

void picchio_greet() {
	int c;
	FILE *fp = fopen("greet.txt", "r");
	while ((c = getc(fp)) != EOF)
        putchar(c);
    fclose(fp);
}

float point_distance (float Ax, float Ay, float Bx, float By) {
	//printf("ax: %d ay: %d bx: %d by: %d\n", Ax, Ay, Bx, By);
	return sqrt( (Ax-Bx)*(Ax-Bx) + (Ay-By)*(Ay-By) );
}

void update_direction(int deg) {
	// printf("dir: %d\n", my_pos.dir);
	// printf("deg: %d\n", deg);
  int d = my_pos.dir + deg;
  if (d > 180) {
    my_pos.dir = ((d - 180) % 360) - 180;
  } else if (d < -180) {
    my_pos.dir = ((d + 180) % 360) + 180;
  } else {
    my_pos.dir = d;
  }
  // printf("Updated direction to %d!\n", my_pos.dir);
}

void update_position(float dist) {
	my_pos.x += dist * sin((my_pos.dir * M_PI) / 180.0);
	my_pos.y += dist * cos((my_pos.dir * M_PI) / 180.0);
}

float time_distance(float time, int speed){
	float d;
	//speed correction
	if(speed==525){
		speed=CORR_DIV2;
	} else if (speed==262){
		speed=CORR_DIV4;
	} else if(speed==131){
		speed=CORR_DIV8;
	} else if(speed==65){
		speed=CORR_DIV16;
	} else {
		d=-1;
	}
	d=(float) (speed*M_PI)/180*WHEEL_RADIUS;
	// printf("Speed: %f\n", d);
	d=d*time;
	// printf("Distance: %f\n", d);
	return d;

}

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
	 float height_ob_up;
	 float height_ob_down;
 };

 void wait_motor_stop(uint8_t motor) {
 	FLAGS_T state;
 	do {
 		get_tacho_state_flags( motor, &state );
 	} while ( state );
 }

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
	set_tacho_speed_sp( motor, speed );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	set_tacho_position_sp( motor, pos );
	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
}

void go_forwards_time(uint8_t *motors, int time, int speed) {
	//int d;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, MOT_DIR * speed );
	set_tacho_speed_sp( motors[0], MOT_DIR * speed * COMP_SX);
	set_tacho_speed_sp( motors[1], MOT_DIR * speed * COMP_DX);
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	//d=(int) time_distance((float) time/1000, speed);
	//update_position(d);
}

void go_backwards_time(uint8_t *motors, int time, int speed) {
	//int d;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], -MOT_DIR * speed * COMP_SX);
	set_tacho_speed_sp( motors[1], -MOT_DIR * speed * COMP_DX);
	multi_set_tacho_time_sp( motors, time );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TIMED );
	//d=(int) time_distance((float) time/1000, speed);
	//update_position(d * -1);
}

void go_forwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	//printf("%f\n", deg);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], speed * COMP_SX);
	set_tacho_speed_sp( motors[1], speed * COMP_DX);
	multi_set_tacho_position_sp( motors, MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	//map((int)my_pos.x, (int)my_pos.y, my_pos.dir, cm, SURE_MISS);
	//update_position(cm);
	wait_motor_stop(motors[0]);
	wait_motor_stop(motors[1]);
}



void go_backwards_cm(uint8_t *motors, int cm, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	//printf("%f\n", deg);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], speed * COMP_SX);
	set_tacho_speed_sp( motors[1], speed * COMP_DX);
	multi_set_tacho_position_sp( motors, -MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	//update_position(cm * -1);
	wait_motor_stop(motors[0]);
	wait_motor_stop(motors[1]);
}

void turn_right(uint8_t *motors, int speed, int deg) {
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	multi_set_tacho_speed_sp( motors, speed );
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	set_tacho_position_sp( motors[0], MOT_DIR*(TURN360*deg)/360 );
	set_tacho_position_sp( motors[1], -MOT_DIR*(TURN360*deg)/360 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	//update_direction(deg);
}

void turn_right_motors(uint8_t *motors, int speed, int deg) {
	turn_right(motors, speed, deg);
	wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
	my_pos.dir = ((((int)my_pos.dir + deg) % 360 ) + 360 ) % 360;
	if (my_pos.dir > 180) {
		my_pos.dir -= 360;
	}
}

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

void turn_left_motors(uint8_t *motors, int speed, int deg) {
	turn_left(motors, speed, deg);
	wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
	my_pos.dir = ((((int)my_pos.dir - deg) % 360 ) + 360 ) % 360;
	if (my_pos.dir > 180) {
		my_pos.dir -= 360;
	}
}

void stop_motors(uint8_t *motors) {
	multi_set_tacho_command_inx( motors, TACHO_STOP );
}

void stop_motor(uint8_t motor) {
	set_tacho_command_inx( motor, TACHO_STOP );
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
	return sum/(float)samples;
}

float get_compass_value_samples(uint8_t compass, int samples) {
	float val, sum = 0;
	int i;
	for (i = 0; i < samples; i++) {
		get_sensor_value0( compass, &val );
		sum  += val;
	}
	float r = sum/samples;
	return r > 180 ? r - 360 : r;
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
	    dir = get_compass_value_samples( compass, 5 );
		} while (dir > 0 || dir < end_dir);
		stop_motors(motors);
	} else {
		int end_dir = start_dir + deg;
		do {
	    dir = get_compass_value_samples( compass, 5 );
		} while (dir < end_dir);
		stop_motors(motors);
	}
	// printf("%d\n", dir);
	//update_direction(deg);
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
	    dir = get_compass_value_samples( compass, 5 );
		} while (dir < 0 || dir > end_dir);
		stop_motors(motors);
	} else {
		int end_dir = start_dir - deg;
		do {
	    dir = get_compass_value_samples( compass, 5 );
		} while (dir > end_dir);
		stop_motors(motors);
	}
	// printf("%d\n", dir);
	//update_direction(-deg);
}

void turn_to_angle(uint8_t *motors, uint8_t gyro, int speed, int deg) {
 	int cur_pos = my_pos.dir;
	int dest = ((deg + 180) % 360 + 360) % 360 - 180;
	if (dest == -180) dest = 180;
	// dest += 6; //TODO
	// if (dest > 180) dest -= 360;
	// printf("Dest: %d\n", dest);
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );

	//printf("dest: %10d SP: %10d a: %10d\n", dest, start_pos, a);

	if (dest > cur_pos) {
		if ((dest - cur_pos) < 180 ) {
			dest -= 6;
			if (dest < -180) dest = 180;
			//printf("turning clockwise\n");
			set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			// while(my_pos.dir < dest);// {
		 	while(cur_pos < dest) {
				cur_pos = my_pos.dir;//get_value_single(gyro);
				// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
				// if (cur_pos > 180) {
				// 	cur_pos = cur_pos - 360;
				// }
				//printf("1 %d\n", cur_pos);
			}
		 	stop_motors(motors);
			//printf("Update of: %d\n", cur_pos-start_pos);
			//update_direction(cur_pos-start_pos);
		}
		else {
			//printf("turning counterclockwise\n");
			dest += 7;
			if (dest > 180) dest = 180;
			set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			while(cur_pos < 0) {
				cur_pos = my_pos.dir;//get_value_single(gyro);
				// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
				// if (cur_pos > 180) {
				// 	cur_pos = cur_pos - 360;
				// }
				// printf("2 %d\n", cur_pos);
			}
			while(cur_pos > dest) {
				cur_pos = my_pos.dir;//get_value_single(gyro);
				// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
				// if (cur_pos > 180) {
				// 	cur_pos = cur_pos - 360;
				// }
				// printf("3 %d\n", cur_pos);
			}
		 	stop_motors(motors);
			//printf("Update of: %d\n", cur_pos-start_pos);
			//update_direction(cur_pos-start_pos);
		}
	}
	else {
		if ((cur_pos - dest) < 180) {
			//printf("turning counterclockwise\n");
			dest += 7;
			if (dest > 180) dest = 180;
			set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			while(cur_pos > dest) {
				cur_pos = my_pos.dir;//get_value_single(gyro);
				// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
				// if (cur_pos > 180) {
				// 	cur_pos = cur_pos - 360;
				// }
				// printf("4 %d\n", cur_pos);
			}
		 	stop_motors(motors);
			//printf("Update of: %d\n", cur_pos-start_pos);
			//update_direction(cur_pos-start_pos);
		}
		else {
			dest -= 6;
			if (dest < -180) dest = 180;
			//printf("turning clockwise\n");
			set_tacho_speed_sp( motors[0], MOT_DIR * speed);
	 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
			multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			while(cur_pos > 0) {
				cur_pos = my_pos.dir;//get_value_single(gyro);
				// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
				// if (cur_pos > 180) {
				// 	cur_pos = cur_pos - 360;
				// }
				// printf("5 %d\n", cur_pos);
			}
			while(cur_pos < dest) {
				cur_pos = my_pos.dir;//get_value_single(gyro);
				// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
				// if (cur_pos > 180) {
				// 	cur_pos = cur_pos - 360;
				// }
				// printf("6 %d\n", cur_pos);
			}
		 	stop_motors(motors);
			//printf("Update of: %d\n", cur_pos-start_pos);
			//update_direction(cur_pos-start_pos);
		}
	}
}

void turn_fix(uint8_t *motors, uint8_t gyro, int speed, int deg) {
 	int cur_pos = my_pos.dir;
	int dest = ((deg + 180) % 360 + 360) % 360 - 180;

 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );

	//printf("dest: %10d SP: %10d a: %10d\n", dest, start_pos, a);

	switch (dest) {
		case 0:
			if (cur_pos > 0) {
				printf("To 0 cc\n");
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
		 		set_tacho_speed_sp( motors[1], MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			 	while(my_pos.dir > 10);// {
					//cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				//}
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed/4);
		 		set_tacho_speed_sp( motors[1], MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > 0);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
			 	stop_motors(motors);
			} else {
				printf("To 0 cw\n");
				set_tacho_speed_sp( motors[0], MOT_DIR * speed);
		 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
			 	while(my_pos.dir < -10);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], MOT_DIR * speed/4);
		 		set_tacho_speed_sp( motors[1], -MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < 0);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
			 	stop_motors(motors);
			}
			break;

		case 90:
			if (cur_pos < -90  || cur_pos > 90) {
				printf("To 90 cc\n");
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
				set_tacho_speed_sp( motors[1], MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < 0 || my_pos.dir > 100);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed/4);
				set_tacho_speed_sp( motors[1], MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > 90) ;//{
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
			} else {
				printf("To 90 cw\n");
				set_tacho_speed_sp( motors[0], MOT_DIR * speed);
				set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < 80);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], MOT_DIR * speed/4);
				set_tacho_speed_sp( motors[1], -MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < 90);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
			}
			break;

		case -180:
			if (cur_pos < 0) {
				printf("To -180 cc\n");
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
				set_tacho_speed_sp( motors[1], MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > -170);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed/4);
				set_tacho_speed_sp( motors[1], MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < 0);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
			} else {
				printf("To -180 cw\n");
				set_tacho_speed_sp( motors[0], MOT_DIR * speed);
				set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < 170);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], MOT_DIR * speed/4);
				set_tacho_speed_sp( motors[1], -MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > 0);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
			}
			break;

		case -90:
			if (cur_pos < 90 && cur_pos > -90) {
				printf("To -90 cc\n");
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
				set_tacho_speed_sp( motors[1], MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > -80);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], -MOT_DIR * speed/4);
				set_tacho_speed_sp( motors[1], MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > -90);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
			} else {
				printf("To -90 cw\n");
				set_tacho_speed_sp( motors[0], MOT_DIR * speed);
				set_tacho_speed_sp( motors[1], -MOT_DIR * speed);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir > 90 || my_pos.dir < -100);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
				set_tacho_speed_sp( motors[0], MOT_DIR * speed/4);
				set_tacho_speed_sp( motors[1], -MOT_DIR * speed/4);
				multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
				while(my_pos.dir < -90);// {
					// cur_pos = my_pos.dir;
					// cur_pos = get_value_single(gyro);
					// cur_pos = (( cur_pos % 360 ) + 360 ) % 360;
					// if (cur_pos > 180) {
					// 	cur_pos = cur_pos - 360;
					// }
				// }
				stop_motors(motors);
			}
			break;

		default:
			turn_to_angle(motors, gyro, speed, deg);
	}
 }

void reinit_pos_gyro(uint8_t *motors, uint8_t gyro, int speed) {
	float start_pos = get_value_samples(gyro,5);
  //printf("start_pos=%f\n", start_pos);
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
  //update_direction(-start_pos);
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
	// printf("start_dir=%f\n", start_dir);
  float dir;
 	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
 	set_tacho_speed_sp( motors[0], -MOT_DIR * speed);
 	set_tacho_speed_sp( motors[1], MOT_DIR * speed);
 	multi_set_tacho_ramp_up_sp( motors, 0 );
 	multi_set_tacho_ramp_down_sp( motors, 0 );
 	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
 	float end_dir = start_dir-deg;
	// printf("end dir=%f\n", end_dir);
 	do {
 	  dir = my_pos.dir;//get_value_single(gyro);
 		// printf("cur_pos=%f\n", dir);
  } while (dir > end_dir);
 	stop_motors(motors);
	//update_direction(-deg);
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
 	  dir = my_pos.dir;//get_value_single(gyro);
 		// printf("cur_pos=%f\n", dir);
 	} while (dir < end_dir);
 	stop_motors(motors);
	//update_direction(deg);
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
	float main_color_val = 5;
	strcpy(main_color, "BLACK");
	if(color_val->r > 5 && color_val->r > 3*color_val->g && color_val->r > 3*color_val->b){
  	float main_color_val = color_val->r;
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

int get_color(uint8_t color, char * buf){
  rgb color_val;
	get_color_values(&color_val, color);
	//printf("r = %f, g = %f, b = %f\n", color_val.r, color_val.g, color_val.b);
	return get_main_color(&color_val, buf);
}

int front_obstacle(uint8_t dist) {
	  int distance;
		if ((distance = (int)get_value_samples(dist, 3)) <= DIST_THRESHOLD )
			return distance;
		else
			return 0;
}

// int go_forwards_obs(uint8_t *motors, uint8_t dist, int cm, int speed) {
//         float d, x, time;
//         struct timeb t0, t1;
//         int count=0, sum = 0;
//         multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
//         set_tacho_speed_sp( motors[0], MOT_DIR * speed * COMP_SX);
//         set_tacho_speed_sp( motors[1], MOT_DIR * speed * COMP_DX);
//         multi_set_tacho_ramp_up_sp( motors, 0 );
//         multi_set_tacho_ramp_down_sp( motors, 0 );
//         multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
//         ftime(&t0);
//         do {
//                 d = front_obstacle(dist);
//                 if(count == 10 && (d==0 || d>cm*10)){
//                         ftime(&t1);
//                         time=(t1.time-t0.time)+(t1.millitm-t0.millitm)/1000.0;
//                         ftime(&t0);
//                         x=time_distance(time, speed)*100;
//                         map_fix((int)my_pos.x, (int)my_pos.y, my_pos.dir, x, SURE_MISS);
//                         //update_position(x);
// 												sum += x;
// 												// printf("%d\n", sum);
//                         count=0;
//                 }
//                 //printf("%f\n", time);
//                 count++;
//         } while ( d == 0 || d > cm*10 );
//         stop_motors(motors);
//         ftime(&t1);
//         time= t1.time - t0.time + ((float) t1.millitm-t0.millitm)/1000;
//         // printf("Time: %f\n", time);
//         x=time_distance(time, speed)*100;
//         map_fix((int)my_pos.x, (int)my_pos.y, my_pos.dir, x, SURE_MISS);
//         //update_position(x);
// 				sum += x;
// 				// printf("%d\n", sum);
// 				float delta = sum*(0.1*sum/100);
// 				map_fix((int)my_pos.x, (int)my_pos.y, my_pos.dir, delta, SURE_MISS);
// 				//update_position(delta);
// 				return sum+delta;
// }

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

int go_forwards_obs(uint8_t *motors, uint8_t motor_head ,uint8_t dist, uint8_t touch, int cm, int speed) {
	int c, d, p, ret = 0;
	pthread_t scanner;
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], MOT_DIR * speed * COMP_SX);
	set_tacho_speed_sp( motors[1], MOT_DIR * speed * COMP_DX);
	multi_set_tacho_ramp_up_sp( motors, 0 );
	multi_set_tacho_ramp_down_sp( motors, 0 );
	multi_set_tacho_command_inx( motors, TACHO_RUN_FOREVER );
	pthread_create( &scanner, NULL, scan_around, (void *)&motor_head);
	do {
		d = front_obstacle(dist);
		get_tacho_position(motor_head, &p);
		c = get_value_single(touch);
		// printf("%d\n", c);
	} while ((d == 0 || d > cm*10.0) && c == 0);
	if (c == 0) {
		ret = p;
		printf("%d\n", p);
	}
	stop_motors(motors);
	pthread_cancel(scanner);
	turn_motor_to_pos(motor_head, speed, 0);
	wait_motor_stop(motor_head);
	//map_fix(my_pos.x, my_pos.y, my_pos.dir, x, SURE_MISS);
	//update_position((int) x);
	return ret;
}

int check_ball(uint8_t dist, uint8_t color, int angle) {
	char s[10];
 	int d = front_obstacle(dist);
 	float x = my_pos.x, y = my_pos.y;
 	get_color(color, s);
	//printf("%s\n", s);
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

void scan_for_obstacle_N_pos(uint8_t *motors, uint8_t dist, uint8_t gyro, int* obstacles, int* angles, int pos, int span, int final_dir, int sp) {
	 int dir, d;
	 int angle, i;
	 float anglef;
	 dir = (int)get_value_single(gyro) % 360;
	 //printf("initial direction %d\n", dir);
	 anglef = (float)(span) / (pos-1);
	 angle = (int) (anglef);
	 d = front_obstacle(dist);
	 obstacles[(pos-1)/2] = d;
	 //check_ball(d, my_pos.dir);
	 angles[(pos-1)/2] = 0;
	 if (final_dir == 1) {
	  for (i=0;i<((pos-1)/2);i++) {
		  turn_left_gyro (motors, gyro, sp, angle);
		  d = front_obstacle(dist);
			obstacles[((pos-1)/2)-(i+1)] = d;
		  angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
			//check_ball(d, my_pos.dir);
	  }
		turn_right_gyro(motors, gyro, MAX_SPEED/16, span/2);
	  //turn_to_angle (motors, gyro, MAX_SPEED/16, dir);
	  for (i=(((pos-1)/2)+1);i<pos;i++) {
		  turn_right_gyro (motors, gyro, sp, angle);
			d = front_obstacle(dist);
		  obstacles[i] = d;
		  angles[i] = (int)((i)*anglef-(span/2));
			//check_ball(d, my_pos.dir);
	  }
	 }
	 if (final_dir == -1) {
		 for (i=(((pos-1)/2)+1);i<pos;i++) {
			 turn_right_gyro (motors, gyro, sp, angle);
			 d = front_obstacle(dist);
			 obstacles[i] = d;
			 angles[i] = (int)((i)*anglef-(span/2));
			 //check_ball(d, my_pos.dir);
		 }
		 // printf("%d\n", dir);
		 turn_left_gyro(motors, gyro, MAX_SPEED/16, span/2);
		 //turn_to_angle (motors, gyro, MAX_SPEED/16, dir);
		 for (i=0;i<((pos-1)/2);i++) {
			 turn_left_gyro (motors, gyro, sp, angle);
			 d = front_obstacle(dist);
			 obstacles[((pos-1)/2)-(i+1)] = d;
			 angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
			 //check_ball(d, my_pos.dir);
		 }
	 }
	 if (final_dir == 0) {
		 for (i=0;i<((pos-1)/2);i++) {
			 turn_left_gyro (motors, gyro, sp, angle);
			 d =
			 obstacles[((pos-1)/2)-(i+1)] = front_obstacle(dist);
			 angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
			 //check_ball(d, my_pos.dir);
		 }
		 turn_right_gyro(motors, gyro, MAX_SPEED/16, span/2);
		 //turn_to_angle (motors, gyro, MAX_SPEED/16, dir);
		 for (i=(((pos-1)/2)+1);i<pos;i++) {
			 turn_right_gyro (motors, gyro, sp, angle);
			 d = front_obstacle(dist);
			 obstacles[i] = d;
			 angles[i] = (int)((i)*anglef-(span/2));
			 //check_ball(d, my_pos.dir);
		 }
		 turn_to_angle (motors, gyro, MAX_SPEED/16, dir);
	 }
 }

 void scan_for_obstacle_N_pos_head(uint8_t motor, uint8_t dist, int* obstacles, int* angles, int pos, int span, int speed) {
	 int i;
	 float anglef;
	 anglef = (float)(span) / (pos-1);
	 printf("%f\n", anglef);
	 // angle = (int) (anglef);
	 turn_motor_to_pos(motor, speed, 0);
	 wait_motor_stop(motor);
	 obstacles[(pos-1)/2] = front_obstacle(dist);
	 angles[(pos-1)/2] = 0;
	 for (i=0;i<((pos-1)/2);i++) {
		 //turn_motor_deg(motor, MAX_SPEED/16, angle);
		 turn_motor_to_pos(motor, speed, (i+1)*anglef);
		 wait_motor_stop(motor);
		 obstacles[((pos-1)/2)-(i+1)] = front_obstacle(dist);
		 angles[((pos-1)/2)-(i+1)] = (int)((i+1)*(-1)*anglef);
	 }
	 //turn_motor_deg(motor, MAX_SPEED/16, ((-1)*(span/2)));
	 turn_motor_to_pos(motor, speed, 0);
	 wait_motor_stop(motor);
	 for (i=(((pos-1)/2)+1);i<pos;i++) {
		 //turn_motor_deg(motor, MAX_SPEED/16, ((-1)*angle));
		 turn_motor_to_pos(motor, speed, (-1*i)*anglef+(span/2));
		 wait_motor_stop(motor);
		 obstacles[i] = front_obstacle(dist);
		 angles[i] = (int)((i)*anglef-(span/2));
	 }
	 //turn_motor_deg(motor, MAX_SPEED/16, (span/2));
	 turn_motor_to_pos(motor, speed, 0);
	 wait_motor_stop(motor);
 }

void turn_motor_obs_to_pos_down(uint8_t motor, int speed, float height_ob){
	float pos = 0;
	set_tacho_stop_action_inx( motor, STOP_ACTION );
 	set_tacho_speed_sp( motor, speed *  MOT_DIR );
 	set_tacho_ramp_up_sp( motor, 0 );
 	set_tacho_ramp_down_sp( motor, 0 );
	if(height_ob<3.5 && height_ob!=0) {
		pos = 120 - ((acos(0.5 - height_ob/ARM_LENGTH)*180/M_PI)-60);
		// printf("pos = %f\n", pos);
	} else if(height_ob>=3.5 || height_ob==0){
	  pos = 60;
	}
	pos=-pos;
 	set_tacho_position_sp( motor, pos );
 	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
 }

 void turn_motor_obs_to_pos_up(uint8_t motor, int speed, float height_ob){
  set_tacho_stop_action_inx( motor, STOP_ACTION );
	set_tacho_speed_sp( motor, speed *  MOT_DIR );
	set_tacho_ramp_up_sp( motor, 0 );
	set_tacho_ramp_down_sp( motor, 0 );
	float pos;
	pos=-atan(height_ob/3.5)*180/M_PI;
	//printf("pos = %f\n", pos);
	set_tacho_position_sp( motor, pos );
	set_tacho_command_inx( motor, TACHO_RUN_TO_ABS_POS );
}

void angle_recal(uint8_t *motors, uint8_t dist, uint8_t gyro, int speed, int th) {
	int start = (int)front_obstacle(dist);
	if (start == 0) {
		printf("I see nothing!\n");
		return;
	}
	printf("I see %d\n", start);
	int i,j;
	int values[100];

	turn_left_gyro(motors, gyro, speed, th);

	for (i = 0; i < th/2; i++) {
		turn_right_gyro(motors, gyro, speed, 2);
		values[i] = (int)front_obstacle(dist);
		printf("I see %d\n", values[i]);
	}
	printf("Done clockwise\n");
	for (j = i; j < th; j++) {
		turn_right_gyro(motors, gyro, speed, 2);
		values[j] = (int)front_obstacle(dist);
		printf("I see %d\n", values[j]);
	}
}

void angle_recal2(uint8_t *motors, uint8_t head, uint8_t dist, uint8_t gyro, int samples, int deg, int th, pthread_mutex_t *gyro_lock) {
	int dir = my_pos.dir;
	if (abs(dir) > th) {
		return;
	}
	int cnt = 0, best = 0;
	int i;
	int d, min = 3000.0;
  int pos = ((samples-1)/2)*deg*-1;
	// printf("%d\n", pos);
	for (i = 0; i < samples; i++) {
		turn_motor_to_pos(head, MAX_SPEED/32, -pos);
		wait_motor_stop(head);
		d = front_obstacle(dist);
		printf("Dist: %d %d\n", d, pos);
		if (d < min && d != 0) {
			min = d;
			best = pos;
			cnt = 1;
		}
		else if (d == min) {
			cnt++;
		}
		pos += deg;
	}
	best = (best*2 + (cnt-1)*deg)/2;
	turn_motor_to_pos(head, MAX_SPEED/32, 0);
	wait_motor_stop(head);
	printf("%d %d\n", best,cnt);
	if (cnt == 0 ){//|| abs(best) < 7) {
	 	return;
	}
	if (best > 0) {
		turn_right(motors, MAX_SPEED/16, best);
		wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
		my_pos.dir = 0;
		// turn_right_gyro(motors, gyro, MAX_SPEED/16, best);
	} else {
		turn_right(motors, MAX_SPEED/16, best);
		wait_motor_stop(motors[0]); wait_motor_stop(motors[1]);
		my_pos.dir = 0;
		// turn_left_gyro(motors, gyro, MAX_SPEED/16, -best);
	}
	millisleep(50);
	pthread_mutex_lock(gyro_lock);
	set_gyro(gyro);
	pthread_mutex_unlock(gyro_lock);
	millisleep(50);
}

void * release_obs_routine(void * thread_args){

	struct obstacle_thread_arguments * args;
	args = (struct obstacle_thread_arguments *) thread_args;
	int motor = args->motor;
	uint8_t motors[2];
	motors[0] = args->motor0;
	motors[1] = args->motor1;
	int speed = args->speed;
	int height_ob_down = args->height_ob_down;
	int height_ob_up = args->height_ob_up;

	int x = (int)my_pos.x, y = (int)my_pos.y;
  // printf("im here...\n");
	turn_motor_obs_to_pos_down(motor, speed, height_ob_down);
	wait_motor_stop(motor);
	// printf("%d, %d\n", x, y);
	// printf("%d, %d\n", x-SIDEX_OBSTACLE/2, y-TAIL-SIDEY_OBSTACLE);
	add_my_obstacle(x-SIDEX_OBSTACLE/2, y-TAIL-SIDEY_OBSTACLE, x+SIDEX_OBSTACLE/2, y-TAIL);
	go_forwards_cm(motors, 5, speed*2);
	turn_motor_obs_to_pos_up(motor, speed, height_ob_up);
	wait_motor_stop(motor);

		return NULL;

}

int go_forwards_cm_obs(uint8_t *motors, uint8_t motor_head, uint8_t dist, uint8_t touch, int cm, int max_dist, int speed) {
	float deg = (360.0*cm*10)/(M_PI*WHEEL_DIAM);
	FLAGS_T state0, state1;
	int c, d, p, ret = 0;
	pthread_t scanner;
	//printf("%f\n", deg);
	multi_set_tacho_stop_action_inx( motors, STOP_ACTION );
	set_tacho_speed_sp( motors[0], speed * COMP_SX);
	set_tacho_speed_sp( motors[1], speed * COMP_DX);
	multi_set_tacho_position_sp( motors, MOT_DIR * deg );
	multi_set_tacho_ramp_up_sp( motors, MOV_RAMP_UP );
	multi_set_tacho_ramp_down_sp( motors, MOV_RAMP_DOWN );
	multi_set_tacho_command_inx( motors, TACHO_RUN_TO_REL_POS );
	pthread_create( &scanner, NULL, scan_around, (void *)&motor_head);

	do {
		get_tacho_state_flags( motors[0], &state0 );
		get_tacho_state_flags( motors[1], &state1 );
		d = front_obstacle(dist);
		get_tacho_position(motor_head, &p);
		c = get_value_single(touch);
	} while ((d == 0 || d > max_dist*10.0) && c == 0 && state0 && state1);

	if (state0 || state1) {
		ret = 1;
	}

	stop_motors(motors);
	pthread_cancel(scanner);
	turn_motor_to_pos(motor_head, speed, 0);
	wait_motor_stop(motor_head);
	return ret;
}
