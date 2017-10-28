#include "config.h"
#include "picchio_lib.c"

void turn_motor_time(uint8_t sn, int speed, int time, int ramp_up, int ramp_down);

void turn_motor_deg(uint8_t sn, int speed, int deg);

void turn_motor_to_pos(uint8_t sn, int speed, int pos);

void go_forwards(uint8_t *sn, int time, int speed);

void turn_right(uint8_t *sn, int speed, int deg);

void turn_left(uint8_t *sn, int speed, int deg);

void wait_motor_stop(uint8_t sn);

float get_value_single(uint8_t sn);

float get_value_samples(uint8_t sn, int samples);
