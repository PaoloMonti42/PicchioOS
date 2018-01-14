#include "config.h"
#include "picchio_lib.c"

void picchio_greet();

float point_distance (float Ax, float Ay, float Bx, float By);

void wait_motor_stop(uint8_t motor);

void turn_motor_deg(uint8_t motor, int speed, int deg);

void turn_motor_to_pos(uint8_t motor, int speed, int pos);

void go_forwards_cm(uint8_t *motors, int cm, int speed);

void go_backwards_cm(uint8_t *motors, int cm, int speed);

void turn_right(uint8_t *motors, int speed, int deg);

void turn_right_motors(uint8_t *motors, int speed, int deg);

void turn_left(uint8_t *motors, int speed, int deg);

void turn_left_motors(uint8_t *motors, int speed, int deg);

void stop_motor(uint8_t motor);

void stop_motors(uint8_t *motors);

float get_value_single(uint8_t sn);

float get_value_samples(uint8_t sn, int samples);

void turn_to_angle(uint8_t *motors, int speed, int deg);

void set_gyro(uint8_t gyro);

void turn_left_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg);

void turn_right_gyro(uint8_t *motors, uint8_t gyro, int speed, int deg);

void get_color_values(rgb *color_val, uint8_t color);

int get_color(uint8_t color, char * buf);

int front_obstacle (uint8_t dist);

void * scan_around(void * arg);

int go_forwards_obs(uint8_t *motors, uint8_t motor_head, uint8_t dist, uint8_t touch, int cm, int speed);

int check_ball(uint8_t dist, uint8_t color, int angle);

void scan_for_obstacle_N_pos (uint8_t *motors, uint8_t dist, uint8_t gyro, int* obstacles, int* angles, int pos, int span, int final_dir, int sp);

void scan_for_obstacle_N_pos_head (uint8_t motor, uint8_t dist, int* obstacles, int* angles, int pos, int span, int speed);

void turn_motor_obs_to_pos_down(uint8_t motor, int speed, float pos_down);

void turn_motor_obs_to_pos_up(uint8_t motor, int speed, float pos_up);

void angle_recal(uint8_t *motors, uint8_t head, uint8_t dist, uint8_t gyro, int samples, int deg, int th, pthread_mutex_t *gyro_lock);

void * release_obs_routine(void * thread_args);

int go_forwards_cm_obs(uint8_t *motors, uint8_t motor_head, uint8_t dist, uint8_t touch, int cm, int max_dist, int speed);

int panic(uint8_t *motors, pthread_mutex_t *pos_lock);
