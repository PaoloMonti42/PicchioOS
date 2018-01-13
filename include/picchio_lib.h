#include "config.h"
#include "picchio_lib.c"

void picchio_greet();

float point_distance (float Ax, float Ay, float Bx, float By);

void turn_motor_time(uint8_t motor, int speed, int time, int ramp_up, int ramp_down);

void turn_motor_deg(uint8_t motor, int speed, int deg);

void turn_motor_deg_compass(uint8_t motor, int speed, int deg);

void turn_motor_to_pos(uint8_t motor, int speed, int pos);

void go_forwards_time(uint8_t *motors, int time, int speed);

void go_backwards_time(uint8_t *motors, int time, int speed);

void go_forwards_cm(uint8_t *motors, int cm, int speed);

void go_backwards_cm(uint8_t *motors, int cm, int speed);

void turn_right(uint8_t *motors, int speed, int deg);

void turn_left(uint8_t *motors, int speed, int deg);

void turn_right_compass(uint8_t *motors, uint8_t compass, int speed, int deg);

void turn_left_compass(uint8_t *motors, uint8_t compass, int speed, int deg);

void wait_motor_stop(uint8_t motor);

void stop_motors(uint8_t *motors);

void stop_motor(uint8_t motor);

float get_value_single(uint8_t sn);

float get_value_samples(uint8_t sn, int samples);

float get_compass_value_samples(uint8_t compass, int samples);

void update_position(float dist);

void get_color_values(rgb *color_val, uint8_t color);

void turn_right_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg);

void turn_left_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg);

void reinit_pos_gyro(uint8_t *motors, uint8_t gyro, int speed);

int translate_angle(int angle, int a);

void turn_to_angle(uint8_t *motors, uint8_t gyro, int speed, int deg);

int get_main_color(rgb *color_val, char * main_color2);

int get_color(uint8_t color, char * buf);

int front_obstacle (uint8_t dist);

int check_ball(uint8_t dist, uint8_t color, int angle);

int go_forwards_obs(uint8_t *motors, uint8_t motor_head, uint8_t dist, uint8_t touch, int cm, int speed);

void scan_for_obstacle_N_pos (uint8_t *motors, uint8_t dist, uint8_t gyro, int* obstacles, int* angles, int pos, int span, int final_dir, int sp);

void scan_for_obstacle_N_pos_head (uint8_t motor, uint8_t dist, int* obstacles, int* angles, int pos, int span, int speed);

void init_gyro(uint8_t *motors, uint8_t gyro, int speed);

void set_gyro(uint8_t gyro);

void turn_motor_obs_to_pos_down(uint8_t motor, int speed, float pos_down);

void turn_motor_obs_to_pos_up(uint8_t motor, int speed, float pos_up);

void angle_recal(uint8_t *motors, uint8_t dist, uint8_t gyro, int speed, int th);

//void release_obs_routine(int motor, uint8_t * motors, int speed, float height_ob_up, float height_ob_down);
